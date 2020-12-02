#include <vector>
#include <matioCpp/matioCpp.h>

using namespace std;

int main(int argc, char *argv[])
{
  matioCpp::File file = matioCpp::File::Create("test_int.mat");
  matioCpp::File file_double = matioCpp::File::Create("test_double.mat");
  matioCpp::File file_3d = matioCpp::File::Create("test_3d.mat");

  // Create a matio matrix of ints 2x2 (from vector)
  vector<int> in = {2, 4, 7, 10};
  matioCpp::MultiDimensionalArray<int> out("test", {2,2}, in.data());

  // Create a matio matrix of ints 2x2x2 (from vector)
  vector<int> in_3d = {2, 4, 7, 9, 10, 13, 21, 24};
  matioCpp::MultiDimensionalArray<int> out_3d("test", {2,2,2}, in_3d.data());

  // create a matio matrix of doubles
  vector<double> in_double = {2.0, 4.0, 6.0, 8.0};
  matioCpp::MultiDimensionalArray<double> out_double("test", {2,2}, in_double.data());

  // adding another 2x2 matrix to an existing matrix
  out_double.resize({2, 2, 2}); // clears previous data
  out_double({0,0,0}) = in_double[0];
  out_double({0,1,0}) = in_double[1];
  out_double({0,0,1}) = in_double[2];
  out_double({0,1,1}) = in_double[3];
  out_double({1,0,0}) = 40.0;
  out_double({1,1,0}) = 50.0;
  out_double({1,0,1}) = 60.0;
  out_double({1,1,1}) = 70.0;
  // {#vectors_vectors, #vectors, #element}
  
  file.write(out);
  file_double.write(out_double);
  file_3d.write(out_3d);

  return 0;
}
