#ifndef FILEMANAGEMENT_H 
#define FILEMANAGEMENT_H 
#include <QDomDocument>
#include <QDomElement>
#include <QSet>
#include <QHash>
#include <QMap>

#include <boolapi.hxx>
#include <curextnd.hxx>
#include <cstrapi.hxx>

#include "hoopsview.h"


BODY* load_acis_model (QString file_path);
void save_acis_entity(ENTITY *entity, const char * file_name);
void save_acis_entity_list(ENTITY_LIST ilistEntity, const char * file_name);
bool parse_xml_file (QString xml_path, QString &file_type, QString &data_name, 
std::vector<std::pair<QString, QString> > &path_pairs);


#endif