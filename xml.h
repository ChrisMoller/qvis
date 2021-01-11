
#define xml_def(v,p) XML_ ## v
enum XML_enums
  {
#include "XMLtags.def"
  };
#undef xml_def


//void handle_qvis (QXmlStreamReader &stream, int idx);

//#define xml_def(v,p) #v, p, XML_ ## v
#define xml_def(v,p) #v, nullptr, XML_ ## v
xml_tag_s xml_tags[] = {
#include "XMLtags.def"
  };
