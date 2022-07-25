#include <string>
#include <vector>

class ByteBuffer;

bool FileExists(const std::string& filename);
int  FileWriteFromBuffer(ByteBuffer& inBuffer, const std::string& filename);
int  FileWriteFromBuffer(std::vector<uint8_t>& inBuffer, const std::string& filename);
int  FileWriteFromString(std::string& inString, const std::string& filename);
int  FileReadToBuffer(ByteBuffer& outBuffer, const std::string& filename);
int  FileReadToBuffer(std::vector<uint8_t>& outBuffer, const std::string& filename);
int  FileReadToString(std::string& outString, const std::string& filename);
