/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sampleClass.I
 * @author drose
 * @date 2000-06-10
 */

/**
 *
 */
INLINE SampleClass::
SampleClass(const SampleClass &copy) :
  _public_data_member(copy._public_data_member),
  _private_data_member(copy._private_data_member),
  _flag(copy._flag)
{
}

/**
 *
 */
INLINE SampleClass::
~SampleClass() {
}

/**
 * A few sentences describing the purpose of flag, the legal values it may be
 * set to, and the implications of setting it to certain values.
 */
INLINE void SampleClass::
set_flag(int flag) {
  _flag = flag;
}

/**
 * A few sentences describing the purpose of flag, the legal values it may be
 * set to, and the implications of setting it to certain values.
 */
INLINE int SampleClass::
get_flag() const {
  return _flag;
}
