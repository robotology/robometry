/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#include <vector>
#include <matioCpp/matioCpp.h>

using namespace std;

int main(int argc, char *argv[])
{
  matioCpp::File file = matioCpp::File::Create("test_int.mat");
  matioCpp::File file_double = matioCpp::File::Create("test_double.mat");

  // Create a matio vector of ints
  vector<int> in = {2, 4, 7, 10};
  matioCpp::Vector<int> out("test");
  out = in;  //"in" is automatically converted to span!

  // create a matio vector of doubles
  vector<double> in_double = {2.0, 4.0, 6.0, 8.0};
  matioCpp::Vector<double> out_double("test");
  out_double = in_double;  //"in" is automatically converted to span!
  
  file.write(out);
  file_double.write(out_double);

  return 0;
}
