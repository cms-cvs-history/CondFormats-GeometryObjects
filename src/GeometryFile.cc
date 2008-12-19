#include "CondFormats/GeometryObjects/interface/GeometryFile.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include <iostream>
#include <fstream>
#include <string>
#include <zlib.h>

GeometryFile::GeometryFile(const std::string & fname, bool zip, unsigned int isize) {
  compressed = zip;
  if (isize==0) isize= computeFileSize(fname);
  blob.reserve(isize);
  read(fname);
}
GeometryFile::GeometryFile(std::istream& is, bool zip, unsigned int isize) {
  compressed = zip;
  if (isize==0) isize= computeStreamSize(is);
  blob.reserve(isize);
  read(is);
}

void GeometryFile::read(std::istream & is) {
  if(compressed){
    std::vector<unsigned char> in;
    in.reserve(isize);
    char c;
    while (is.get(c))
      in.push_back((unsigned char)c);
    blob.resize(isize);
    uLongf destLen = blob.size();
    int zerr =  compress2(&*blob.begin(), &destLen,
                          &*in.begin(), in.size(),
                          9);
    if (zerr!=0) edm::LogError("GeometryFile")<< "Compression error" << zerr;
    blob.resize(destLen);  
  }else{
    char c;
    while (is.get(c))
      blob.push_back( (unsigned char)c);
    blob.resize(blob.size());
  }
}


void GeometryFile::write(std::ostream & os) const {
  if(compressed){
    std::vector<unsigned char> out(isize);
    uLongf destLen = out.size();
    int zerr =  uncompress(&*out.begin(),  &destLen,
                           &*blob.begin(), blob.size());
    if (zerr!=0 || out.size()!=destLen) 
      edm::LogError("GeometryFile")<< "uncompressing error " << zerr
                                   << " original size was " << isize
                                   << " new size is " << destLen;
    os.write((const char *)(&*out.begin()),out.size());
  }else{
    os.write((char *)&*blob.begin(),blob.size());
  }
}



void GeometryFile::read(const std::string & fname) {
     std::ifstream ifile(fname.c_str());
     if (!ifile) { edm::LogError("GeometryFile")<< "file " << fname << " does not exist...";}
     else read(ifile);
     ifile.close();
}
 
void GeometryFile::write(const std::string & fname) const {
  std::ofstream ofile(fname.c_str());
  write(ofile);
  ofile.close();
}

unsigned int GeometryFile::computeFileSize(const std::string & fname) {
  unsigned int is=0;
  std::ifstream ifile(fname.c_str());
  if (!ifile) { edm::LogError("GeometryFile")<< "file " << fname << " does not exist...";}
  else is = computeStreamSize(ifile);
  ifile.close();
  return is;
}

unsigned int GeometryFile::computeStreamSize(std::istream & is) {
  unsigned int rs=0;
  char c;
  while (is.get(c)) rs++;
  is.clear();
  is.seekg(0);
  return rs;
}
