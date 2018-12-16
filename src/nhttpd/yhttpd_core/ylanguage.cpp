//=============================================================================
// YHTTPD
// Language
//=============================================================================

// c
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include <fstream>
#include <iostream>

#include <system/helpers.h>

// yhttpd
#include <yconfig.h>
#include <yhttpd.h>
#include "ylanguage.h"
#include "yconnection.h"

#include <global.h>

//=============================================================================
// Instance Handling - like Singelton Pattern
//=============================================================================
//-----------------------------------------------------------------------------
// Init as Singelton
//-----------------------------------------------------------------------------
CLanguage* CLanguage::instance = NULL;
CConfigFile* CLanguage::DefaultLanguage = NULL;
CConfigFile* CLanguage::ConfigLanguage = NULL;
CConfigFile* CLanguage::NeutrinoLanguage = NULL;
std::string CLanguage::language = "";
std::string CLanguage::language_dir = "";
//-----------------------------------------------------------------------------
// There is only one Instance
//-----------------------------------------------------------------------------
CLanguage *CLanguage::getInstance(void){
	if (!instance)
		instance = new CLanguage();
	return instance;
}

//-----------------------------------------------------------------------------
void CLanguage::deleteInstance(void){
	if (instance)
		delete instance;
	instance = NULL;
}

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CLanguage::CLanguage(void)
{
	DefaultLanguage = new CConfigFile(',');
	ConfigLanguage = new CConfigFile(',');
	NeutrinoLanguage = new CConfigFile(',');
	language = "";
	language_dir =getLanguageDir();
}

//-----------------------------------------------------------------------------
CLanguage::~CLanguage(void)
{
	delete DefaultLanguage;
	delete ConfigLanguage;
	delete NeutrinoLanguage;
}

//=============================================================================

//-----------------------------------------------------------------------------
void CLanguage::setLanguage(std::string _language){
	language=_language;
	ConfigLanguage->loadConfig(language_dir + "/" + _language);
	DefaultLanguage->loadConfig(language_dir + "/" + HTTPD_DEFAULT_LANGUAGE);

	const char * path[2] = { LOCALEDIR_VAR, LOCALEDIR };
	for (int i = 0; i < 2; i++)
	{
		std::string filename = path[i];
		filename += "/";
		filename += g_settings.language;
		filename += ".locale";

		if(access(filename.c_str(), F_OK) == 0) {
			NeutrinoLanguage->loadConfig(filename, ' ');
			break;
		}
		else if (i == 1) {
			// load neutrino default language (should not happen)
			NeutrinoLanguage->loadConfig(LOCALEDIR "/english.locale", ' ');
		}
	}
}

//-----------------------------------------------------------------------------
// return translation for "id" if not found use default language
//-----------------------------------------------------------------------------
std::string CLanguage::getTranslation(std::string id){
	std::string trans=ConfigLanguage->getString(id,"");
	if(trans.empty())
		trans=NeutrinoLanguage->getString(id,"");
	if(trans.empty())
		trans=DefaultLanguage->getString(id,"");
	if (trans.empty()) {
		write_missing_trans(id);
		trans = "# " + id + " #";
	}
	return trans;
}
void CLanguage::write_missing_trans(std::string const& entry)
{
const char *file = "/tmp/yweb.missing.translation";
std::ifstream in;
std::ofstream out;
std::string line;
bool found = false;

	in.open(file, std::ios::in);
	if(in.is_open())
	{
		while (std::getline(in, line) && !found)
		{
			if (line.compare(entry) == 0)
				found = true;
		}
		in.close();
	}
	if (!found)
	{
		out.open(file, std::ios::out|std::ios::app);
		if (out.is_open())
		{
			out << entry << std::endl;
			out.close();
		}
	}
}
//-----------------------------------------------------------------------------
// Find language directory
//-----------------------------------------------------------------------------
std::string CLanguage::getLanguageDir(void){
	std::string tmpfilename = "/"+Cyhttpd::ConfigList["Language.directory"],dir="";

	if( access(Cyhttpd::ConfigList["WebsiteMain.override_directory"] + tmpfilename,R_OK) == 0)
		dir = Cyhttpd::ConfigList["WebsiteMain.override_directory"] + tmpfilename;
	else if(access(Cyhttpd::ConfigList["WebsiteMain.directory"] + tmpfilename,R_OK) == 0)
		dir = Cyhttpd::ConfigList["WebsiteMain.directory"] + tmpfilename;
	return dir;
}

