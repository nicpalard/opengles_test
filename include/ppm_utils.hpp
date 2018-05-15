#ifndef _PPM_UTILS_H_
#define _PPM_UTILS_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

inline unsigned char* load_ppm(char* file_name, uint &width, uint &height)
{
  std::ifstream file(file_name);
  std::string magic_number;
  uint max_value;
  file >> magic_number >> width >> height >> max_value;
  if (magic_number != "P6")
	throw std::invalid_argument("Current PPM Image format is " + magic_number  + " and must be P6");
  if (max_value > 255)
	throw std::invalid_argument("Max value is " + std::to_string(max_value) + "but it should not be > 255");

  size_t size = width * height * 3;
  unsigned char* data = new unsigned char[size];
  file.read((char*)(&data[0]), size);
  return data;
}

inline void write_ppm(char* file_name, unsigned char* data, int width, int height)
{
  std::ofstream file(file_name);
  file << "P6" << "\n"
	   << width << "\n"
	   << height << "\n"
	   << 255 << "\n";
  size_t size = width * height * 3;
  file.write((char*)(&data[0]), size);
}

inline float* uchar_to_float(unsigned char* data, uint size)
{
  float* float_data = new float[size];
  for (uint i = 0 ; i < size ; ++i)
  {
	float_data[i] = static_cast<float>(data[i]);
  }
  return float_data;
}

inline unsigned char* float_to_uchar(float* data, uint size)
{
  unsigned char* uchar_data = new unsigned char[size];
  for (uint i = 0 ; i < size ; i++)
  {
	float value = data[i];
	if (value < 0) value = 0;
	if (value > 255) value = 255;
	uchar_data[i] = static_cast<float>(data[i]);
  }
  return uchar_data;
}

inline unsigned char* rgb_to_gray(unsigned char* data, uint width, uint height)
{
  unsigned char* data_gray = new unsigned char[width * height];
  int cpt = 0;
  for (uint i = 0 ; i < width * height * 3; i+=3)
  {
	data_gray[cpt++] = (data[i] + data[i+1] + data[i+2]) / 3;
  }
  
  return data_gray;
}

inline unsigned char* gray_to_rgb(unsigned char* data, uint width, uint height)
{
  unsigned char* data_rgb = new unsigned char[width * height * 3];
  int cpt = 0;
  for (uint i = 0 ; i < width * height; ++i)
  {
	data_rgb[cpt++] = data[i];
	data_rgb[cpt++] = data[i];
	data_rgb[cpt++] = data[i];
  }
  
  return data_rgb;
}

#endif