
#define xml_def(v,p) XML_ ## v
enum XML_enums
  {
#include "XMLtags.def"
  };
#undef xml_def


#if 1
extern xml_tag_s xml_tags[];
#else
#define xml_def(v,p) #v, nullptr, XML_ ## v
xml_tag_s xml_tags[] = {
#include "XMLtags.def"
  };
#endif
