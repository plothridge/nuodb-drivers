#include "dbdimp.h"

DBISTATE_DECLARE;

void dbd_init(dbistate_t* dbistate)
{
	DBISTATE_INIT;  /*  Initialize the DBI macros  */
	DBIS = dbistate;
}

int dbd_db_login6_sv(SV *dbh, imp_dbh_t *imp_dbh, SV *dbname, SV *uid, SV *pwd, SV *attr)
{

	SV* sv;
	SV** svp;
	HV* hv;
	STRLEN db_len;

	// Obtain the private attributes that are stashed in imp_dbh by DBI::NuoDB::connect
	sv = DBIc_IMP_DATA(imp_dbh);
	
	if (!sv || !SvROK(sv)) {
		return FALSE;
	}

	hv = (HV*) SvRV(sv);
	if (SvTYPE(hv) != SVt_PVHV)
		return FALSE;

	NuoDB::Connection *conn = createConnection();
	imp_dbh->conn = conn;

	NuoDB::Properties *properties = conn->allocProperties();
	
	if (SvOK(uid))
		properties->putValue("user", SvPV_nolen(uid));

	if (SvOK(pwd))
		properties->putValue("password", SvPV_nolen(pwd));

	if ((svp = hv_fetch(hv, "schema", 6, FALSE)))
		properties->putValue("schema", SvPV(*svp, db_len));
		
	try {
		conn->openDatabase(SvPV_nolen(dbname), properties);
                DBIc_IMPSET_on(imp_dbh);
	} catch (NuoDB::SQLException& xcp) {
		do_error(dbh, xcp.getSqlcode(), (char *) xcp.getText());
		conn->close();
		return FALSE;
	}

	return TRUE;
}

int dbd_st_prepare(SV *sth, imp_sth_t *imp_sth, char *statement, SV *attribs)
{
	D_imp_dbh_from_sth;

	try {
		imp_sth->pstmt = imp_dbh->conn->prepareStatement(statement);
		DBIc_IMPSET_on(imp_sth);
	} catch (NuoDB::SQLException& xcp) {
		do_error(sth, xcp.getSqlcode(), (char *) xcp.getText());
		return FALSE;
	}
	
	return TRUE;
}

int dbd_st_execute(SV* sth, imp_sth_t* imp_sth)
{
	SV **statement;
	STRLEN slen;

	statement = hv_fetch((HV*) SvRV(sth), "Statement", 9, FALSE);
	char * str_ptr = SvPV(*statement, slen);

	try {
		if (
			!strstr(str_ptr, "SELECT") &&
			!strstr(str_ptr, "select")
		) {
			DBIc_ACTIVE_off(imp_sth);
			imp_sth->rs = NULL;
			return imp_sth->pstmt->execute();
		} else {
			NuoDB::ResultSet *rs = imp_sth->pstmt->executeQuery();
			imp_sth->rs = rs;
	
			NuoDB::ResultSetMetaData *md = rs->getMetaData();
			DBIc_NUM_FIELDS(imp_sth) = md->getColumnCount();
		}
	} catch (NuoDB::SQLException& xcp) {
		do_error(sth, xcp.getSqlcode(), (char *) xcp.getText());
		return FALSE;
	}
	
	return TRUE;
}

AV* dbd_st_fetch(SV *sth, imp_sth_t* imp_sth)
{
	AV* av;
	int i;

	NuoDB::ResultSet *rs = imp_sth->rs;

	if (!rs) {
		return Nullav;
	}

	int numFields = DBIc_NUM_FIELDS(imp_sth);

	if (!rs->next()) {
		DBIc_ACTIVE_off(imp_sth);
		return Nullav;
	}

	av = DBIc_DBISTATE(imp_sth)->get_fbav(imp_sth);

	for (i = 0; i < numFields; i++) {
		SV *sv = AvARRAY(av)[i];

		const char * str = rs->getString(i + 1);

		if (rs->wasNull()) {
			(void) SvOK_off(sv);
		} else {
			sv_setpvn(sv, str, strlen(str));
		}
	}
	
	return av;
}


void dbd_st_destroy(SV *sth, imp_sth_t *imp_sth) {

	if (imp_sth->rs)
		imp_sth->rs->close();

	imp_sth->pstmt->close();

	DBIc_IMPSET_off(imp_sth);
}

int dbd_st_finish(SV* sth, imp_sth_t* imp_sth)
{
	return TRUE;
}


int dbd_db_commit(SV* dbh, imp_dbh_t* imp_dbh)
{
	try {
		imp_dbh->conn->commit();
	} catch (NuoDB::SQLException& xcp) {
		do_error(dbh, xcp.getSqlcode(), (char *) xcp.getText());
		return FALSE;
	}

	return TRUE;
}

int dbd_db_rollback(SV* dbh, imp_dbh_t* imp_dbh)
{
	try {
		imp_dbh->conn->rollback();
	} catch (NuoDB::SQLException& xcp) {
		do_error(dbh, xcp.getSqlcode(), (char *) xcp.getText());
		return FALSE;
	}

	return TRUE;
}

SV* dbd_db_FETCH_attrib(SV *dbh, imp_dbh_t *imp_dbh, SV *keysv)
{
        STRLEN kl;
        char *key = SvPV(keysv, kl);

        if (kl==10 && strEQ(key, "AutoCommit")) {
		return sv_2mortal(boolSV(DBIc_has(imp_dbh, DBIcf_AutoCommit)));
	} else {
		return Nullsv;
	}
}

int dbd_db_STORE_attrib(SV* dbh, imp_dbh_t* imp_dbh, SV* keysv, SV* valuesv)
{
	STRLEN kl;
	char *key = SvPV(keysv, kl);
	bool bool_value = SvTRUE(valuesv);

	if (kl==10 && strEQ(key, "AutoCommit")) {
		imp_dbh->conn->setAutoCommit(bool_value);
		DBIc_set(imp_dbh, DBIcf_AutoCommit, bool_value);
		return TRUE;
	} else {
		return FALSE;
	}
}

SV* dbd_st_FETCH_attrib(SV *sth, imp_sth_t *imp_sth, SV *keysv)
{
	return Nullsv;
}

int dbd_st_STORE_attrib(SV *sth, imp_sth_t *imp_sth, SV *keysv, SV *valuesv)
{
	return FALSE;
}


int dbd_st_blob_read (SV *sth, imp_sth_t *imp_sth, int field, long offset, long len, SV *destrv, long destoffset)
{
	/* quell warnings */
	sth= sth;
	imp_sth=imp_sth;
	field= field;
	offset= offset;
	len= len;
	destrv= destrv;
	destoffset= destoffset;
	return FALSE;
}


int dbd_db_disconnect(SV* dbh, imp_dbh_t* imp_dbh)
{
	imp_dbh->conn->close();

	return TRUE;
}

int dbd_bind_ph (SV *sth, imp_sth_t *imp_sth, SV *param, SV *value, IV sql_type, SV *attribs, int is_inout, IV maxlen)
{
	croak("Bind parameters are not supported.");
}

void dbd_db_destroy(SV* dbh, imp_dbh_t* imp_dbh)
{
	imp_dbh->conn->release();
}

void do_error(SV* h, int rc, char* what)
{
        D_imp_xxh(h);

        sv_setiv(DBIc_ERR(imp_xxh), (IV)rc);

        SV *errstr = DBIc_ERRSTR(imp_xxh);
        sv_setpv(errstr, what);
}
