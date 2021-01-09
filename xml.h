
#define xml_def(v) XML_ ## v
enum XML_enums
  {
#include "XMLtags.def"
  };
#undef xml_def

typedef struct {
  const QString tag;
  int idx;
} xml_tag_s;

#define xml_def(v) #v, XML_ ## v
xml_tag_s xml_tags[] = {
#include "XMLtags.def"
  };
