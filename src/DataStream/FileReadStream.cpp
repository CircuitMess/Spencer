#include "FileReadStream.h"

FileReadStream::FileReadStream(SerialFlashFile source) : source(source){
	buffer.reserve(bufferSize);
}

bool FileReadStream::available(){
	return !buffer.empty() || source.position() < source.size();
}

unsigned char FileReadStream::get(){
	if(buffer.empty()){
		buffer.resize(bufferSize);
		source.read(buffer.data(), buffer.size());
	}

	if(buffer.empty()) return 0;

	unsigned char byte = buffer.back();
	buffer.pop_back();
	return byte;
}