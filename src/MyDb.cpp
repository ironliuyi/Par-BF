#include "config.h"
#include "MyDb.h"
#include <vector>
// Class constructor. Requires a path to the location
// where the database is located, and a database name
DbEnv* MyDb::dbenv=NULL;
void errCallback (const DbEnv *env, const char *prefix,
const char *errMsg)
{
	std::cout << prefix << " " << errMsg << std::endl;
}

int getSecKey(Db *sdbp,const Dbt *pkey,
		const Dbt *pdata,Dbt *skey)
{

	// First, extract the structure contained in the primary's data
	DB_ITEM *db_item= (DB_ITEM *)pdata->get_data();
	// Now set the secondary key's data to be the representative's name
	skey->set_data(&(db_item->skey));
	skey->set_size(sizeof(db_item->skey));
	// Return 0 to indicate that the record can be created/updated.
	return (0);
}
// Class constructor. Requires a path to the location
// where the database is located, and a database name
MyDb::MyDb(const std::string &path, const std::string &dbName)
:
		// Instantiate Db object
dbFileName_(path + "/"+dbName),secdbFileName_(dbFileName_ + "_Indexing"),// Database file name
cFlags_(DB_CREATE)
// If the database doesn't yet exist,
// allow it to be created.
{
	if(NULL==dbenv)
	{
		dbenv=new DbEnv(0);
		dbenv->set_errpfx("env_erro");
		//dbenv->set_errcall(this->errCallback);
		dbenv->open(path.c_str(),DB_CREATE|DB_INIT_MPOOL,0);

	}
	try
	{
		db_=new Db(dbenv,0);
// Redirect debugging information to std::cerr
		db_->set_error_stream(&std::cerr);
// Open the database
		db_->open(NULL, dbFileName_.c_str(), NULL, DB_BTREE, cFlags_, 0);
//		secdb_ = new Db(dbenv, 0);
//// Redirect debugging information to std::cerr
//		secdb_->set_error_stream(&std::cerr);
//// Open the database
//		secdb_->open(NULL, secdbFileName_.c_str(), NULL, DB_BTREE, cFlags_, 0);
//		db_->associate(0, secdb_,getSecKey, 0);
	}
// DbException is not a subclass of std::exception, so we
// need to catch them both.
	catch(DbException &e)
	{
		std::cerr << "Error opening database: " << dbFileName_ << "\n";
		std::cerr << e.what() << std::endl;
	}
	catch(std::exception &e)
	{
		std::cerr << "Error opening database: " << dbFileName_ << "\n";
		std::cerr << e.what() << std::endl;
	}
	buildSecIndex();
}
void MyDb::buildSecIndex()
{

	try
	{
		secdb_ = new Db(dbenv, 0);
// Redirect debugging information to std::cerr
		secdb_->set_error_stream(&std::cerr);
		secdb_->set_flags(DB_DUPSORT);
// Open the database
		secdb_->open(NULL, secdbFileName_.c_str(), NULL, DB_BTREE, cFlags_, 0);
		db_->associate(0, secdb_,getSecKey, 0);
	}
// DbException is not a subclass of std::exception, so we
// need to catch them both.
	catch(DbException &e)
	{
		std::cerr << "Error opening indexing database: " <<secdbFileName_ << "\n";
		std::cerr << e.what() << std::endl;
	}
	catch(std::exception &e)
	{
		std::cerr << "Error opening indexing database: " << secdbFileName_ << "\n";
		std::cerr << e.what() << std::endl;
	}
}
// Private member used to close a database. Called from the class
// destructor.
void
MyDb::close()
{
// Close the db
	try
	{
		secdb_->close(0);
		delete secdb_;
		std::cout << "Database " << secdbFileName_
				<< " is closed." << std::endl;
		db_->close(0);
		std::cout << "Database " << dbFileName_
		<< " is closed." << std::endl;
		delete db_;
	}
	catch(DbException &e)
	{
		std::cerr << "Error closing database: " << dbFileName_ << "\n";
		std::cerr << e.what() << std::endl;
	}
	catch(std::exception &e)
	{
		std::cerr << "Error closing database: " << dbFileName_ << "\n";
		std::cerr << e.what() << std::endl;
	}
}
void MyDb::putItem(const KEY_TYPE& key,DB_ITEM* value)
{
	Dbt key_;
	memset(&key_,0,sizeof(Dbt));
	key_.set_data((void*)&key);
	key_.set_size(sizeof(KEY_TYPE));
	Dbt storing_value;
	memset(&storing_value,0,sizeof(Dbt));
	storing_value.set_data(value);
	storing_value.set_size(sizeof(DB_ITEM)+(value->data_len)+1);
	int ret;
	ret=db_->put(0,&key_,&storing_value,DB_NOOVERWRITE);
//	delete val;
	if(ret==DB_KEYEXIST)
	{
		db_->err(ret,"");
	}
}
uint64_t MyDb::getItem(const KEY_TYPE& key, string& value) const
{
	uint64_t segID;
	Dbc *cursorp;
	try{
		db_->cursor(NULL,&cursorp,0);
		Dbt key_;
		memset(&key_, 0, sizeof(Dbt));
		key_.set_data((void*)&key);
		key_.set_size(sizeof(KEY_TYPE));
		Dbt stored_value;
		memset(&stored_value, 0, sizeof(Dbt));
		// Iterate over the database, deleting each record in turn.
		int ret;
		if((ret = cursorp->get(&key_, &stored_value, DB_SET)) == 0) {
			DB_ITEM* result=(DB_ITEM*) (stored_value.get_data());
//			char *tempres=new char[result->data_len+1];
//			value=strcpy(tempres,result->dataptr);
			value.append(result->dataptr,result->data_len);
			segID=result->skey;
//			delete[] tempres;
		}
		else
		{
			DEBUG_PRINT("%s\n","NOT FOUND VALUE");
			return 0;
		}
	} catch(DbException &e) {
		db_->err(e.get_errno(), "Error!");
	} catch (std::exception &e) {
		db_->errx("Error! %s", e.what());
	}
	// Cursors must be closed
	if (cursorp != NULL)
		cursorp->close();
	return segID;
}
void MyDb::deleteItem(const KEY_TYPE& key)
{
	Dbc *cursorp;
	try{
		db_->cursor(NULL,&cursorp,0);
		Dbt key_((void*)&key,sizeof(KEY_TYPE));
		Dbt data;
		// Iterate over the database, deleting each record in turn.
		int ret;
		if((ret = cursorp->get(&key_, &data,
		DB_SET)) == 0) {
		cursorp->del(0);
	}
	} catch(DbException &e) {
		db_->err(e.get_errno(), "Error!");
	} catch (std::exception &e) {
		db_->errx("Error! %s", e.what());
	}
	// Cursors must be closed
	if (cursorp != NULL)
	cursorp->close();
}
void MyDb::getDataFromSeckey(uint64_t& sid,vector<pair<KEY_TYPE,string> >& vec)
{
	Dbc *cursorp;
	try{
		secdb_->cursor(NULL, &cursorp, 0);
		Dbt skey_;
		memset(&skey_,0,sizeof(Dbt));
		skey_.set_data(&sid);
		skey_.set_size(sizeof(sid));
		Dbt pdata; // Primary key and data
		int ret = cursorp->get(&skey_, &pdata, DB_SET);
		string data_s;
		if(!ret)
		{
			do {
				DB_ITEM* final_result = (DB_ITEM*) (pdata.get_data());
				data_s.append(final_result->dataptr,final_result->data_len);
				vec.push_back(make_pair(final_result->pkey,data_s));
				data_s.clear();
			}while (cursorp->get(&skey_, &pdata, DB_NEXT_DUP) == 0);
		}
	}catch(DbException &e) {
		secdb_->err(e.get_errno(), "Error!");
	} catch (std::exception &e) {
		secdb_->errx("Error! %s", e.what());
	}
	cursorp->close();
	return;
}
void MyDb::deleteSegment(const KEY_TYPE& key)
{
	Dbc *cursorp;
	string value;
	uint64_t seg_id=getItem(key,value);
	if(value.c_str()!=NULL)
	{
		try {
			secdb_->cursor(NULL, &cursorp, 0);
			Dbt skey_;
			memset(&skey_, 0, sizeof(Dbt));
			skey_.set_data(&seg_id);
			skey_.set_size(sizeof(seg_id));
			Dbt pdata; // Primary key and data
			int ret = cursorp->get(&skey_, &pdata, DB_SET);
			if (!ret) {
				do {
					cursorp->del(0);
				} while (cursorp->get(&skey_, &pdata, DB_NEXT_DUP) == 0);
			}
		} catch (DbException &e) {
			secdb_->err(e.get_errno(), "Error!");
		} catch (std::exception &e) {
			secdb_->errx("Error! %s", e.what());
		}
		cursorp->close();
	}
	return;
}
