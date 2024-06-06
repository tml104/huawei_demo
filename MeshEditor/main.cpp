#include "stdafx.h"
#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QTextCodec>

#include "HDB.h"
#include "ha_bridge.h"
#include "license.hxx"
#include "ckoutcom.hxx"
#include "kernapi.hxx"
#include "spa_unlock_result.hxx"


HDB * m_pHDB=0; 

void initAcisHoops ()
{
	base_configuration base_config;
	logical ok = initialize_base( &base_config);
	const char *unlock_str = 
		"3VKKZ85TA8QRNSJT3HK3EC5NE82ZMCTV9CB7VCTXVA3CGCHWER5D2ZHHTC732HTWS8PCPGM"
		"RPAKUKNMZ983DNGJQTXECKUHX98NZVWJZR8ADPJHWAAECPHTTVCZDPG5V7AHDDZHR7ABZPS"
		"JZ78VDV2H83RLC2HJTV8MDDEMRVAKUENH3PCDRJH5WACSZKCHVR3K3AZH8VRRZAQ5TE8QKP"
		"Z82TVHLDW8M9VQ3VJN7QHACS8XJTRGM7Y8NW8CNEQD3PGM59DHZ3HJ3AZ5FHXPKZH5NHAN5"
		"WWJNPVT5AZMUQHDDE8M6TRG59C2MRG359D2B7VCAEQD8TECFFHXLPEDFWD2B7VQKE85TN8Q"
		"3JHBBKEGCSU2DKRRFWAXNA8QREZ5TH8B3ADHXKCTDJYHA9L3UNDJR9LACKZH8R3KKVQD89R"
		"HCKSJZRC4CN8JHVC4CNZJ3R3KKVQD89RHCKSJZRC4CN8JHVC4CNAH5R3KKVQD89RHCKSJZR"
		"C4CN8JHVC4CNAJ8R3KKVQD89RHCKSJZRC4CKNP33R4CN8JZR3K7VQD89RHCKSJZRC4CN8JH"
		"VC4CMYHZ9RCAEE5NJR6ZKCH8PADUBZJ3TRMUBQTHKRCAEN5NJR6ZKCH8PADUBUJZVCAUB85"
		"ZPRMZDJQ7A8QRMYHZPAECKHP6TA5CD8P6V8HUJAP83CJUDATTH823AUJXKATZMCHR9LTZ28"
		"HH9L2CDJTTH873AUJXKATZMCHR9LVCK8TH9LTZKZJ8VCADZQ5NACTF7EH29EQKKJQSQHTZY"
		"QX5KVX5AEHTS8QRKNXSKA359C8NA8N3EQMBQ8K7LQMSA3D3AQPUREELU82TPE75AZMSA8Q7"
		"EETC78P7D85LRAXDKAMCKCSKJNPRV8NCMSH57RZZGEMR7ATDMCJZVAPZ2N53P83C2WJ8P8M"
		"RPZHFNRGUJSJ3TAMCVHHV3XHDEJH3VRCCPAM5PCTCPSH3KAAUPCH5VRKCNGJ8PA8UPAHWEC"
		"3UDG5US8AZPEJWNCUUKNM57AUZA85LV8KCJJTTH8QRNS2M7CTMW8BJ9VG5AQ5TH8QRNSJT7CTMW8BJ9VG5AQ5T" ;

	spa_unlock_result out = spa_unlock_products( unlock_str );
	if ( out.get_state() == SPA_UNLOCK_FAIL ) {
		QMessageBox::warning(NULL, QObject::tr("License Invalid"), QObject::tr("The key provided for ACIS/InterOp is invalid. The application may not work.\nPlease add your unlock code from your spa_unlock_xxx.cpp to CDetectModifyApp::InitInstance function."));
	}
	else if ( out.get_state() == SPA_UNLOCK_PASS_WARN ) {
		char str[256];
		sprintf(str,"%s\n This evaluation key will expire in %i days.", out.get_message_text(), out.get_num_days());		
		QMessageBox::warning(NULL,str, "Spatial Eval Warning");
	}
	outcome o;
	o = api_start_modeller(0);
	check_outcome(o);
	o = api_initialize_hoops_acis_bridge();
	check_outcome(o);

	m_pHDB = new HDB(); 
	m_pHDB->Init();
	m_pHDB->SetDriverType("opengl");	
}

void termAcisHoops ()
{
	delete m_pHDB;
	api_terminate_hoops_acis_bridge();
	api_stop_modeller();
	terminate_base();
}

int main(int argc, char *argv[])
{
	HC_Define_System_Options ("license = (customer = internal_release_license,"
		"product = (im, kanji, classic), key = 3F221C5C-EBFD529-1384D57F-380D0DBF)");


	QApplication a(argc, argv);
	QTextCodec *codec = QTextCodec::codecForName("GB2312");  

	QTextCodec::setCodecForLocale(codec);
	QTextCodec::setCodecForCStrings(codec);  
	QTextCodec::setCodecForTr(codec);
	//QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF8"));

	initAcisHoops();





	MainWindow *w = new MainWindow ();
	w->show();
	int ret = a.exec();
	//

	//
	delete w;
	termAcisHoops();
	return ret;
}
