#include "StdAfx.h"
#include "FileManagement.h"

BODY * load_acis_model (QString file_path)
{
	FILE *f = fopen (file_path.toAscii ().data (), "r");
	if (!f){
		QMessageBox::critical (NULL, QObject::tr ("打开错误！"), QObject::tr("打开模型文件失败！"));
		return NULL;
	}
	ENTITY_LIST entities;
	api_restore_entity_list (f, TRUE, entities);
	for (int i = 0; i != entities.count (); ++i)
	{
		ENTITY *ent = entities[i];
		if (is_BODY (ent))
		{
			//api_clean_entity(ent);
			return (BODY *)ent;
		}
	}
	return NULL;
}

void save_acis_entity(ENTITY *entity, const char * file_name)
{
	ENTITY_LIST elist;
	ENTITY *copyentity = NULL;
	api_deep_down_copy_entity (entity, copyentity);
	elist.add (copyentity);
	// Set the units and product_id.
	FileInfo fileinfo;
	fileinfo.set_units (1.0);
	fileinfo.set_product_id ("Example Application");
	outcome result = api_set_file_info((FileId_ | FileUnits), fileinfo);
	check_outcome(result); 

	// Also set the option to produce sequence numbers in the SAT file.
	result = api_set_int_option("sequence_save_files", 1);
	check_outcome(result);

	// Open a file for writing, save the list of entities, and close the file.
	FILE * save_file = fopen(file_name, "w");
	result = api_save_entity_list(save_file, TRUE, elist);
	fclose(save_file);
	check_outcome(result);
}

void save_acis_entity_list(ENTITY_LIST ilistEntity, const char * file_name)
{
	if(ilistEntity.count()<1) return;

	//deep copy entity
	ENTITY_LIST elist; elist.clear();
	for(int i=0;i<ilistEntity.count();i++)
	{
		ENTITY *pCopyEntity = NULL;
		api_deep_down_copy_entity (ilistEntity[i], pCopyEntity);
		elist.add (pCopyEntity);
	}
	// Set the units and product_id.
	FileInfo fileinfo;
	fileinfo.set_units (1.0);
	fileinfo.set_product_id ("Example Application");
	outcome result = api_set_file_info((FileId_ | FileUnits), fileinfo);
	check_outcome(result); 

	// Also set the option to produce sequence numbers in the SAT file.
	result = api_set_int_option("sequence_save_files", 1);
	check_outcome(result);

	// Open a file for writing, save the list of entities, and close the file.
	FILE * save_file = fopen(file_name, "w");
	result = api_save_entity_list(save_file, TRUE, elist);
	fclose(save_file);
	check_outcome(result);
}

bool parse_xml_file (QString xml_path, QString &file_type, QString &data_name, std::vector<std::pair<QString, QString> > &path_pairs)
{
	QDomDocument doc("mydocument");
	QFile file(xml_path);
	if (!file.open(QIODevice::ReadOnly))
		return false;
	if (!doc.setContent(&file)) {
		file.close();
		return false;
	}
	file.close();
	QDomElement root_elem = doc.documentElement();
	file_type = root_elem.tagName ();
	data_name = root_elem.attribute ("name");
	QDomNode n = root_elem.firstChild();
	std::pair<QString, QString> path_pair;

	if (file_type != "meshedit" && file_type != "meshmatch" && file_type != "meshoptimization"){
		QMessageBox::warning (NULL, QObject::tr ("解析错误！"), QObject::tr ("这不是一个有效的文件！"));
		return false;
	}
	QFileInfo fileInfo (xml_path);
	QString xml_file_dir = fileInfo.absolutePath ();

	while (!n.isNull ()){
		QDomElement e = n.toElement();
		path_pair.first = xml_file_dir + "/" + e.attribute ("meshpath");
		path_pair.second = xml_file_dir + "/" + e.attribute ("modelpath");
		path_pairs.push_back (path_pair);
		n = n.nextSibling ();
	}

	return true;
}