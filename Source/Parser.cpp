#include <Source/Headers.h>

#include <Source/TextDoc.h>
#include <Source/TokenizedFile.h>

void parseTextDoc(const TextDoc& doc);

// ============================================================================
int main(int argc, char* argv[])
{
  if (argc != 2) {
    return -1;
  }
  
  TextDoc doc = loadTextDoc(argv[1]);

  parseTextDoc(doc);

  return 0;
}
