
#define xml_def(v) XML_ ## v
enum XML_enums
  {
#include "XMLtags.def"
  };


#undef xml_def
#define xml_def(v) #v
const char *XML_tags[] = {
#include "XMLtags.def"
  };
