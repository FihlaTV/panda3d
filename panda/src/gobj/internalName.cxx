// Filename: internalName.cxx
// Created by: masad (15Jul04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"
#include "internalName.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "preparedGraphicsObjects.h"

PT(InternalName) InternalName::_root;
PT(InternalName) InternalName::_error;
PT(InternalName) InternalName::_default;
PT(InternalName) InternalName::_vertex;
PT(InternalName) InternalName::_normal;
PT(InternalName) InternalName::_tangent;
PT(InternalName) InternalName::_binormal;
PT(InternalName) InternalName::_texcoord;
PT(InternalName) InternalName::_color;
PT(InternalName) InternalName::_transform_blend;

TypeHandle InternalName::_type_handle;
TypeHandle InternalName::_texcoord_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: InternalName::Constructor
//       Access: Private
//  Description: Use make() to make a new InternalName instance.
////////////////////////////////////////////////////////////////////
InternalName::
InternalName(InternalName *parent, const string &basename) :
  _parent(parent),
  _basename(basename)
{
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
InternalName::
~InternalName() {
  if (_parent != (const InternalName *)NULL) {
    NameTable::iterator ni = _parent->_name_table.find(_basename);
    nassertv(ni != _parent->_name_table.end());
    _parent->_name_table.erase(ni);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::append
//       Access: Published
//  Description: Constructs a new InternalName based on this name,
//               with the indicated string following it.  This is a
//               cheaper way to construct a hierarchical name than
//               InternalName::make(parent->get_name() + ".basename").
////////////////////////////////////////////////////////////////////
PT(InternalName) InternalName::
append(const string &name) {
  if (name.empty()) {
    return this;
  }

  size_t dot = name.rfind('.');
  if (dot != string::npos) {
    return append(name.substr(0, dot))->append(name.substr(dot + 1));
  }

  NameTable::iterator ni = _name_table.find(name);
  if (ni != _name_table.end()) {
    return (*ni).second;
  }

  PT(InternalName) internal_name = new InternalName(this, name);
  _name_table[name] = internal_name;
  return internal_name;
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::get_name
//       Access: Published
//  Description: Returns the complete name represented by the
//               InternalName and all of its parents.
////////////////////////////////////////////////////////////////////
string InternalName::
get_name() const {
  if (_parent == get_root()) {
    return _basename;

  } else if (_parent == (InternalName *)NULL) {
    return string();

  } else {
    return _parent->get_name() + "." + _basename;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void InternalName::
output(ostream &out) const {
  if (_parent == get_root()) {
    out << _basename;

  } else if (_parent == (InternalName *)NULL) {
    out << "(root)";

  } else {
    _parent->output(out);
    out << '.' << _basename;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a InternalName object
////////////////////////////////////////////////////////////////////
void InternalName::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
  BamReader::get_factory()->register_factory(_texcoord_type_handle, make_texcoord_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::finalize
//       Access: Public, Virtual
//  Description: Method to ensure that any necessary clean up tasks
//               that have to be performed by this object are performed
////////////////////////////////////////////////////////////////////
void InternalName::
finalize() {
  // Unref the pointer that we explicitly reffed in make_from_bam().
  unref();

  // We should never get back to zero after unreffing our own count,
  // because we expect to have been stored in a pointer somewhere.  If
  // we do get to zero, it's a memory leak; the way to avoid this is
  // to call unref_delete() above instead of unref(), but this is
  // dangerous to do from within a virtual function.
  nassertv(get_ref_count() != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type InternalName is encountered
//               in the Bam file.  It should create the InternalName
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *InternalName::
make_from_bam(const FactoryParams &params) {
  // The process of making a InternalName is slightly
  // different than making other Writable objects.
  // That is because all creation of InternalNames should
  // be done through the make() constructor.
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);

  // The name is the only thing written to the data stream.
  string name = scan.get_string();

  // Make a new InternalName with that name (or get the previous one
  // if there is one already).
  PT(InternalName) me = make(name);

  // But now we have a problem, since we have to hold the reference
  // count and there's no way to return a TypedWritable while still
  // holding the reference count!  We work around this by explicitly
  // upping the count, and also setting a finalize() callback to down
  // it later.
  me->ref();
  manager->register_finalize(me);
  
  return me.p();
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::make_texcoord_from_bam
//       Access: Protected, Static
//  Description: This is a temporary method; it exists only to support
//               old bam files (4.11 through 4.17) generated before we
//               renamed this class from TexCoordName to InternalName.
////////////////////////////////////////////////////////////////////
TypedWritable *InternalName::
make_texcoord_from_bam(const FactoryParams &params) {
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  string name = scan.get_string();
  PT(InternalName) me;
  if (name == "default") {
    me = get_texcoord();
  } else {
    me = get_texcoord_name(name);
  }

  me->ref();
  manager->register_finalize(me);
  
  return me.p();
}

////////////////////////////////////////////////////////////////////
//     Function: InternalName::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void InternalName::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_string(get_name());
}

