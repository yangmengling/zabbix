/* 
** ZABBIX
** Copyright (C) 2000-2005 SIA Zabbix
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**/

#include "common.h"
#include "db.h"
#include "log.h"
#include "zlog.h"
#include "security.h"

#include "evalfunc.h"

/******************************************************************************
 *                                                                            *
 * Function: evaluate_LOGSOURCE                                               *
 *                                                                            *
 * Purpose: evaluate function 'logsource' for the item                        *
 *                                                                            *
 * Parameters: item - item (performance metric)                               *
 *             parameter - ignored                                            *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, result is stored in 'value' *
 *               FAIL - failed to evaluate function                           *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int evaluate_LOGSOURCE(char *value, DB_ITEM *item, char *parameter)
{
	DB_RESULT	result;
	DB_ROW	row;

	char		sql[MAX_STRING_LEN];
	int		now;
	int		res = SUCCEED;

	if(item->value_type != ITEM_VALUE_TYPE_LOG)
	{
		return	FAIL;
	}

	now=time(NULL);

	zbx_snprintf(sql,sizeof(sql),"select source from history_log where itemid=" ZBX_FS_UI64 " order by clock desc",
		item->itemid);

	result = DBselectN(sql,1);
	row = DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)
	{
		zabbix_log(LOG_LEVEL_DEBUG, "Result for LOGSOURCE is empty" );
		res = FAIL;
	}
	else
	{
		if(strcmp(row[0], parameter) == 0)
		{
			strcpy(value,"1");
		}
		else
		{
			strcpy(value,"0");
		}
	}
	DBfree_result(result);

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_LOGSEVERITY                                             *
 *                                                                            *
 * Purpose: evaluate function 'logseverity' for the item                      *
 *                                                                            *
 * Parameters: item - item (performance metric)                               *
 *             parameter - ignored                                            *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, result is stored in 'value' *
 *               FAIL - failed to evaluate function                           *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int evaluate_LOGSEVERITY(char *value, DB_ITEM *item, char *parameter)
{
	DB_RESULT	result;
	DB_ROW		row;

	char		sql[MAX_STRING_LEN];
	int		now;
	int		res = SUCCEED;

	if(item->value_type != ITEM_VALUE_TYPE_LOG)
	{
		return	FAIL;
	}

	now=time(NULL);

	zbx_snprintf(sql,sizeof(sql),"select severity from history_log where itemid=" ZBX_FS_UI64 " order by clock desc",
		item->itemid);

	result = DBselectN(sql,1);
	row = DBfetch(result);
	if(!row || DBis_null(row[0])==SUCCEED)
	{
		zabbix_log(LOG_LEVEL_DEBUG, "Result for LOGSEVERITY is empty" );
		res = FAIL;
	}
	else
	{
		strcpy(value,row[0]);
	}
	DBfree_result(result);

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_COUNT                                                   *
 *                                                                            *
 * Purpose: evaluate function 'count' for the item                            *
 *                                                                            *
 * Parameters: item - item (performance metric)                               *
 *             parameter - number of seconds                                  *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, result is stored in 'value' *
 *               FAIL - failed to evaluate function                           *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int evaluate_COUNT(char *value, DB_ITEM *item, int parameter)
{
	DB_RESULT	result;
	DB_ROW	row;

	char		table[MAX_STRING_LEN];
	int		now;
	int		res = SUCCEED;

	if( (item->value_type != ITEM_VALUE_TYPE_FLOAT) && (item->value_type != ITEM_VALUE_TYPE_UINT64))
	{
		return	FAIL;
	}

	now=time(NULL);

	if(item->value_type == ITEM_VALUE_TYPE_UINT64)
	{
		strscpy(table,"history_uint");
	}
	else
	{
		strscpy(table,"history");
	}
	result = DBselect("select count(value) from %s where clock>%d and itemid=" ZBX_FS_UI64,
		table,
		now-parameter,
		item->itemid);

	row = DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)
	{
		zabbix_log(LOG_LEVEL_DEBUG, "Result for COUNT is empty" );
		res = FAIL;
	}
	else
	{
		strcpy(value,row[0]);
	}
	DBfree_result(result);

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_SUM                                                     *
 *                                                                            *
 * Purpose: evaluate function 'sum' for the item                              *
 *                                                                            *
 * Parameters: item - item (performance metric)                               *
 *             parameter - number of seconds                                  *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, result is stored in 'value' *
 *               FAIL - failed to evaluate function                           *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int evaluate_SUM(char *value, DB_ITEM *item, int parameter, int flag)
{
	DB_RESULT	result;
	DB_ROW	row;

	char		sql[MAX_STRING_LEN];
	char		table[MAX_STRING_LEN];
	int		now;
	int		res = SUCCEED;
	int		rows = 0;
	double		sum=0;
	zbx_uint64_t	sum_uint64=0;
	zbx_uint64_t	value_uint64;

	if( (item->value_type != ITEM_VALUE_TYPE_FLOAT) && (item->value_type != ITEM_VALUE_TYPE_UINT64))
	{
		return	FAIL;
	}

	now=time(NULL);

	if(flag == ZBX_FLAG_SEC)
	{
		if(item->value_type == ITEM_VALUE_TYPE_UINT64)
		{
			strscpy(table,"history_uint");
		}
		else
		{
			strscpy(table,"history");
		}
		result = DBselect("select sum(value) from %s where clock>%d and itemid=" ZBX_FS_UI64,
			table,
			now-parameter,
			item->itemid);

		row = DBfetch(result);
		if(!row || DBis_null(row[0])==SUCCEED)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for SUM is empty" );
			res = FAIL;
		}
		else
		{
			strcpy(value,row[0]);
		}
	}
	else if(flag == ZBX_FLAG_VALUES)
	{
		if(item->value_type == ITEM_VALUE_TYPE_UINT64)
		{
			strscpy(table,"history_uint");
		}
		else
		{
			strscpy(table,"history");
		}
		zbx_snprintf(sql,sizeof(sql),"select value from %s where itemid=" ZBX_FS_UI64 " order by clock desc",
			table,
			item->itemid);
		result = DBselectN(sql, parameter);
		if(item->value_type == ITEM_VALUE_TYPE_UINT64)
		{
			while((row=DBfetch(result)))
			{
				ZBX_STR2UINT64(value_uint64,row[0]);
				sum_uint64+=value_uint64;
				rows++;
			}
			if(rows>0)	zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_UI64, sum_uint64);
		}
		else
		{
			while((row=DBfetch(result)))
			{
				sum+=atof(row[0]);
				rows++;
			}
			if(rows>0)	zbx_snprintf(value,MAX_STRING_LEN, ZBX_FS_DBL, sum);
		}
		if(0 == rows)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for SUM is empty" );
			res = FAIL;
		}
	}
	else
	{
		zabbix_log(LOG_LEVEL_WARNING, "Unknown flag [%d] Expected [%d] or [%d]",
			flag,
			ZBX_FLAG_SEC,
			ZBX_FLAG_VALUES);
		return	FAIL;
	}

	DBfree_result(result);

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_AVG                                                     *
 *                                                                            *
 * Purpose: evaluate function 'avg' for the item                              *
 *                                                                            *
 * Parameters: item - item (performance metric)                               *
 *             parameter - number of seconds                                  *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, result is stored in 'value' *
 *               FAIL - failed to evaluate function                           *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int evaluate_AVG(char *value,DB_ITEM	*item,int parameter,int flag)
{
	DB_RESULT	result;
	DB_ROW		row;

	char		sql[MAX_STRING_LEN];
	int		now;
	int		res = SUCCEED;
	int		rows;
	double		sum=0;

	char		table[MAX_STRING_LEN];

	if( (item->value_type != ITEM_VALUE_TYPE_FLOAT) && (item->value_type != ITEM_VALUE_TYPE_UINT64))
	{
		return	FAIL;
	}

	now=time(NULL);

	if(item->value_type == ITEM_VALUE_TYPE_UINT64)
	{
		strscpy(table,"history_uint");
	}
	else
	{
		strscpy(table,"history");
	}

	if(flag == ZBX_FLAG_SEC)
	{
		result = DBselect("select avg(value) from %s where clock>%d and itemid=" ZBX_FS_UI64,
			table,
			now-parameter,
			item->itemid);

		row = DBfetch(result);
		
		if(!row || DBis_null(row[0])==SUCCEED)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for AVG is empty" );
			res = FAIL;
		}
		else
		{
			strcpy(value,row[0]);
			del_zeroes(value);
		}
	}
	else if(flag == ZBX_FLAG_VALUES)
	{
		zbx_snprintf(sql,sizeof(sql),"select value from %s where itemid=" ZBX_FS_UI64 " order by clock desc",
			table,
			item->itemid);
		result = DBselectN(sql, parameter);
		rows=0;
		while((row=DBfetch(result)))
		{
			sum+=atof(row[0]);
			rows++;
		}
		if(rows == 0)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for AVG is empty" );
			res = FAIL;
		}
		else
		{
			zbx_snprintf(value,MAX_STRING_LEN, ZBX_FS_DBL, sum/(double)rows);
		}
	}
	else
	{
		zabbix_log(LOG_LEVEL_WARNING, "Unknown flag [%d] Expected [%d] or [%d]",
			flag,
			ZBX_FLAG_SEC,
			ZBX_FLAG_VALUES);
		return	FAIL;
	}

	DBfree_result(result);

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_MIN                                                     *
 *                                                                            *
 * Purpose: evaluate function 'min' for the item                              *
 *                                                                            *
 * Parameters: item - item (performance metric)                               *
 *             parameter - number of seconds                                  *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, result is stored in 'value' *
 *               FAIL - failed to evaluate function                           *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int evaluate_MIN(char *value,DB_ITEM	*item,int parameter, int flag)
{
	DB_RESULT	result;
	DB_ROW		row;

	char		sql[MAX_STRING_LEN];
	int		now;
	int		rows;
	int		res = SUCCEED;

	char		table[MAX_STRING_LEN];
	zbx_uint64_t	min_uint64=0;
	zbx_uint64_t	l;

	double		min=0;
	double		f;

	if( (item->value_type != ITEM_VALUE_TYPE_FLOAT) && (item->value_type != ITEM_VALUE_TYPE_UINT64))
	{
		return	FAIL;
	}

	now=time(NULL);

	if(item->value_type == ITEM_VALUE_TYPE_UINT64)
	{
		strscpy(table,"history_uint");
	}
	else
	{
		strscpy(table,"history");
	}

	if(flag == ZBX_FLAG_SEC)
	{
		result = DBselect("select min(value) from %s where clock>%d and itemid=" ZBX_FS_UI64,
			table,
			now-parameter,
			item->itemid);
		row = DBfetch(result);
		if(!row || DBis_null(row[0])==SUCCEED)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for MIN is empty" );
			res = FAIL;
		}
		else
		{
			strcpy(value,row[0]);
			del_zeroes(value);
		}
	}
	else if(flag == ZBX_FLAG_VALUES)
	{
		zbx_snprintf(sql,sizeof(sql),"select value from %s where itemid=" ZBX_FS_UI64 " order by clock desc",
			table,
			item->itemid);
		result = DBselectN(sql,parameter);

		rows=0;
		while((row=DBfetch(result)))
		{
			if(item->value_type == ITEM_VALUE_TYPE_UINT64)
			{
				ZBX_STR2UINT64(l,row[0]);

				if(rows==0)		min_uint64 = l;
				else if(l<min_uint64)	min_uint64 = l;
			}
			else
			{
				f=atof(row[0]);
				if(rows==0)	min = f;
				else if(f<min)	min = f;
			}
			rows++;
		}

		if(rows==0)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for MIN is empty" );
			res = FAIL;
		}
		else
		{
			if(item->value_type == ITEM_VALUE_TYPE_UINT64)
			{
				zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_UI64, min_uint64);
			}
			else
			{
				zbx_snprintf(value,MAX_STRING_LEN, ZBX_FS_DBL, min);
			}
		}
	}
	else
	{
		zabbix_log(LOG_LEVEL_WARNING, "Unknown flag [%d] Expected [%d] or [%d]",
			flag,
			ZBX_FLAG_SEC,
			ZBX_FLAG_VALUES);
		return	FAIL;
	}

	DBfree_result(result);

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_MAX                                                     *
 *                                                                            *
 * Purpose: evaluate function 'max' for the item                              *
 *                                                                            *
 * Parameters: item - item (performance metric)                               *
 *             parameter - number of seconds                                  *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, result is stored in 'value' *
 *               FAIL - failed to evaluate function                           *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int evaluate_MAX(char *value,DB_ITEM *item,int parameter,int flag)
{
	DB_RESULT	result;
	DB_ROW		row;

	char		sql[MAX_STRING_LEN];
	int		now;
	int		res = SUCCEED;
	int		rows;
	double		f;
	double		max;

	char		table[MAX_STRING_LEN];
	zbx_uint64_t	max_uint64=0;
	zbx_uint64_t	l;
	
	zabbix_log( LOG_LEVEL_DEBUG, "In evaluate_MAX()");

	if( (item->value_type != ITEM_VALUE_TYPE_FLOAT) && (item->value_type != ITEM_VALUE_TYPE_UINT64))
	{
		return	FAIL;
	}

	now=time(NULL);

	if(flag == ZBX_FLAG_SEC)
	{
		if(item->value_type == ITEM_VALUE_TYPE_UINT64)
		{
			strscpy(table,"history_uint");
		}
		else
		{
			strscpy(table,"history");
		}
		result = DBselect("select max(value) from %s where clock>%d and itemid=" ZBX_FS_UI64,
			table,
			now-parameter,
			item->itemid);

		row = DBfetch(result);

		if(!row || DBis_null(row[0])==SUCCEED)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for MAX is empty" );
			res = FAIL;
		}
		else
		{
			strcpy(value,row[0]);
			del_zeroes(value);
		}
	}
	else if(flag == ZBX_FLAG_VALUES)
	{
		if(item->value_type == ITEM_VALUE_TYPE_UINT64)
		{
			strscpy(table,"history_uint");
		}
		else
		{
			strscpy(table,"history");
		}
		zbx_snprintf(sql,sizeof(sql),"select value from %s where itemid=" ZBX_FS_UI64 " order by clock desc",
			table,
			item->itemid);
		result = DBselectN(sql,parameter);
		rows=0;
		while((row=DBfetch(result)))
		{
			if(item->value_type == ITEM_VALUE_TYPE_UINT64)
			{
				ZBX_STR2UINT64(l,row[0]);

				if(rows==0)		max_uint64 = l;
				else if(l>max_uint64)	max_uint64 = l;
			}
			else
			{
				f=atof(row[0]);
				if(rows==0)	max=f;
				else if(f>max)	max=f;
			}
			rows++;
		}
		if(rows == 0)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for MAX is empty" );
			res = FAIL;
		}
		else
		{
			if(item->value_type == ITEM_VALUE_TYPE_UINT64)
			{
				zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_UI64, max_uint64);
			}
			else
			{
				zbx_snprintf(value,MAX_STRING_LEN, ZBX_FS_DBL, max);
			}
		}
	}
	else
	{
		zabbix_log(LOG_LEVEL_WARNING, "Unknown flag [%d] Expected [%d] or [%d]",
			flag,
			ZBX_FLAG_SEC,
			ZBX_FLAG_VALUES);
		return	FAIL;
	}

	DBfree_result(result);
	
	zabbix_log( LOG_LEVEL_DEBUG, "End of evaluate_MAX()");

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_DELTA                                                   *
 *                                                                            *
 * Purpose: evaluate function 'delat' for the item                            *
 *                                                                            *
 * Parameters: item - item (performance metric)                               *
 *             parameter - number of seconds                                  *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, result is stored in 'value' *
 *               FAIL - failed to evaluate function                           *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int evaluate_DELTA(char *value,DB_ITEM *item,int parameter, int flag)
{
	DB_RESULT	result;
	DB_ROW		row;

	char		sql[MAX_STRING_LEN];
	int		now;
	int		res = SUCCEED;
	int		rows;
	double		f;
	double		min,max;
	
	zabbix_log( LOG_LEVEL_DEBUG, "In evaluate_DELTA()");

	if(item->value_type != ITEM_VALUE_TYPE_FLOAT)
	{
		return	FAIL;
	}

	now=time(NULL);

	if(flag == ZBX_FLAG_SEC)
	{
		result = DBselect("select max(value)-min(value) from history where clock>%d and itemid=" ZBX_FS_UI64,
			now-parameter,
			item->itemid);

		row = DBfetch(result);
		if(!row || DBis_null(row[0])==SUCCEED)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for DELTA is empty" );
			res = FAIL;
		}
		else
		{
			strcpy(value,row[0]);
			del_zeroes(value);
		}
	}
	else if(flag == ZBX_FLAG_VALUES)
	{
		zbx_snprintf(sql,sizeof(sql),"select value from history where itemid=" ZBX_FS_UI64 " order by clock desc",
			item->itemid);
		result = DBselectN(sql,parameter);
		rows=0;
		while((row=DBfetch(result)))
		{
			f=atof(row[0]);
			if(rows==0)
			{
				min=f;
				max=f;
			}
			else
			{
				if(f>max)	max=f;
				if(f<min)	min=f;
			}
			rows++;
		}
		if(rows==0)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for DELTA is empty" );
			res = FAIL;
		}
		else
		{
			zbx_snprintf(value,MAX_STRING_LEN, ZBX_FS_DBL, max-min);
		}
	}
	else
	{
		zabbix_log(LOG_LEVEL_WARNING, "Unknown flag [%d] Expected [%d] or [%d]",
			flag,
			ZBX_FLAG_SEC,
			ZBX_FLAG_VALUES);
		return	FAIL;
	}

	DBfree_result(result);

	zabbix_log( LOG_LEVEL_DEBUG, "End of evaluate_DELTA()");

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_NODATA                                                  *
 *                                                                            *
 * Purpose: evaluate function 'nodata' for the item                           *
 *                                                                            *
 * Parameters: item - item (performance metric)                               *
 *             parameter - number of seconds                                  *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, result is stored in 'value' *
 *               FAIL - failed to evaluate function                           *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int evaluate_NODATA(char *value,DB_ITEM	*item,int parameter)
{
	int		now;
	int		res = SUCCEED;

	now = time(NULL);

	if(item->lastclock + parameter > now)
	{
		strcpy(value,"0");
	}
	else
	{
		strcpy(value,"1");
	}

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_function                                                *
 *                                                                            *
 * Purpose: evaluate function                                                 *
 *                                                                            *
 * Parameters: item - item to calculate function for                          *
 *             function - function (for example, 'max')                       *
 *             parameter - parameter of the function)                         *
 *             flag - if EVALUATE_FUNCTION_SUFFIX, then include units and     *
 *                    suffix (K,M,G) into result value (for example, 15GB)    *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, value contains its value    *
 *               FAIL - evaluation failed                                     *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
int evaluate_function(char *value,DB_ITEM *item,char *function,char *parameter)
{
	int	ret  = SUCCEED;
	time_t  now;
	struct  tm      *tm;
	
	int	fuzlow, fuzhig;

	int	day;
	int	len;

	zabbix_log( LOG_LEVEL_DEBUG, "In evaluate_function(%s)",
		function);

	if(strcmp(function,"last")==0)
	{
		if(item->lastvalue_null==1)
		{
			ret = FAIL;
		}
		else
		{
			switch (item->value_type) {
				case ITEM_VALUE_TYPE_FLOAT:
					zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_DBL,item->lastvalue_dbl);
					del_zeroes(value);
					break;
				case ITEM_VALUE_TYPE_UINT64:
					zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_UI64,item->lastvalue_uint64);
					break;
				default:
					strcpy(value,item->lastvalue_str);
					break;
			}
		}
	}
	else if(strcmp(function,"prev")==0)
	{
		if(item->prevvalue_null==1)
		{
			ret = FAIL;
		}
		else
		{
			switch (item->value_type) {
				case ITEM_VALUE_TYPE_FLOAT:
					zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_DBL,item->prevvalue_dbl);
					del_zeroes(value);
					break;
				case ITEM_VALUE_TYPE_UINT64:
					zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_UI64,item->prevvalue_uint64);
					break;
				default:
					strcpy(value,item->prevvalue_str);
					break;
			}
		}
	}
	else if(strcmp(function,"min")==0)
	{
		if(parameter[0]=='#')
			ret = evaluate_MIN(value,item,atoi(parameter+1),ZBX_FLAG_VALUES);
		else
			ret = evaluate_MIN(value,item,atoi(parameter),ZBX_FLAG_SEC);
	}
	else if(strcmp(function,"max")==0)
	{
		if(parameter[0]=='#')
			ret = evaluate_MAX(value,item,atoi(parameter+1),ZBX_FLAG_VALUES);
		else
			ret = evaluate_MAX(value,item,atoi(parameter),ZBX_FLAG_SEC);
	}
	else if(strcmp(function,"avg")==0)
	{
		if(parameter[0]=='#')
			ret = evaluate_AVG(value,item,atoi(parameter+1),ZBX_FLAG_VALUES);
		else
			ret = evaluate_AVG(value,item,atoi(parameter),ZBX_FLAG_SEC);
	}
	else if(strcmp(function,"sum")==0)
	{
		if(parameter[0]=='#')
			ret = evaluate_SUM(value,item,atoi(parameter+1),ZBX_FLAG_VALUES);
		else
			ret = evaluate_SUM(value,item,atoi(parameter),ZBX_FLAG_SEC);
	}
	else if(strcmp(function,"count")==0)
	{
		ret = evaluate_COUNT(value,item,atoi(parameter));
	}
	else if(strcmp(function,"delta")==0)
	{
		if(parameter[0]=='#')
			ret = evaluate_DELTA(value,item,atoi(parameter+1),ZBX_FLAG_VALUES);
		else
			ret = evaluate_DELTA(value,item,atoi(parameter),ZBX_FLAG_SEC);
	}
	else if(strcmp(function,"nodata")==0)
	{
		ret = evaluate_NODATA(value,item,atoi(parameter));
	}
	else if(strcmp(function,"date")==0)
	{
		now=time(NULL);
                tm=localtime(&now);
                zbx_snprintf(value,MAX_STRING_LEN,"%.4d%.2d%.2d",
			tm->tm_year+1900,
			tm->tm_mon+1,
			tm->tm_mday);
	}
	else if(strcmp(function,"dayofweek")==0)
	{
		now=time(NULL);
                tm=localtime(&now);
		/* The number of days since Sunday, in the range 0 to 6. */
		day=tm->tm_wday;
		if(0 == day)	day=7;
                zbx_snprintf(value,MAX_STRING_LEN,"%d",
			day);
	}
	else if(strcmp(function,"time")==0)
	{
		now=time(NULL);
                tm=localtime(&now);
                zbx_snprintf(value,MAX_STRING_LEN,"%.2d%.2d%.2d",
			tm->tm_hour,
			tm->tm_min,
			tm->tm_sec);
	}
	else if(strcmp(function,"abschange")==0)
	{
		if((item->lastvalue_null==1)||(item->prevvalue_null==1))
		{
			ret = FAIL;
		}
		else
		{
			switch (item->value_type) {
				case ITEM_VALUE_TYPE_FLOAT:
					zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_DBL,
						(double)abs(item->lastvalue_dbl-item->prevvalue_dbl));
					del_zeroes(value);
					break;
				case ITEM_VALUE_TYPE_UINT64:
					zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_UI64,
						labs(item->lastvalue_uint64-item->prevvalue_uint64));
					break;
				default:
					if(strcmp(item->lastvalue_str, item->prevvalue_str) == 0)
					{
						strcpy(value,"0");
					}
					else
					{
						strcpy(value,"1");
					}
					break;
			}
		}
	}
	else if(strcmp(function,"change")==0)
	{
		if((item->lastvalue_null==1)||(item->prevvalue_null==1))
		{
			ret = FAIL;
		}
		else
		{
			switch (item->value_type) {
				case ITEM_VALUE_TYPE_FLOAT:
					zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_DBL,
						item->lastvalue_dbl-item->prevvalue_dbl);
					del_zeroes(value);
					break;
				case ITEM_VALUE_TYPE_UINT64:
					zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_UI64,
						item->lastvalue_uint64-item->prevvalue_uint64);
					break;
				default:
					if(strcmp(item->lastvalue_str, item->prevvalue_str) == 0)
					{
						strcpy(value,"0");
					}
					else
					{
						strcpy(value,"1");
					}
					break;
			}
		}
	}
	else if(strcmp(function,"diff")==0)
	{
		if((item->lastvalue_null==1)||(item->prevvalue_null==1))
		{
			ret = FAIL;
		}
		else
		{
			switch (item->value_type) {
				case ITEM_VALUE_TYPE_FLOAT:
					if(cmp_double(item->lastvalue_dbl, item->prevvalue_dbl) == 0)
					{
						strcpy(value,"0");
					}
					else
					{
						strcpy(value,"2");
					}
					break;
				case ITEM_VALUE_TYPE_UINT64:
					if(item->lastvalue_uint64 == item->prevvalue_uint64)
					{
						strcpy(value,"1");
					}
					else
					{
						strcpy(value,"0");
					}
					break;
				default:
					if(strcmp(item->lastvalue_str, item->prevvalue_str) == 0)
					{
						strcpy(value,"0");
					}
					else
					{
						strcpy(value,"1");
					}
					break;
			}
/*			if( (item->value_type==ITEM_VALUE_TYPE_FLOAT) || (item->value_type==ITEM_VALUE_TYPE_UINT64))
			{
				if(cmp_double(item->lastvalue, item->prevvalue) == 0)
				{
					strcpy(value,"0");
				}
				else
				{
					strcpy(value,"1");
				}
			}
			else
			{
				if(strcmp(item->lastvalue_str, item->prevvalue_str) == 0)
				{
					strcpy(value,"0");
				}
				else
				{
					strcpy(value,"1");
				}
			}*/
		}
	}
	else if(strcmp(function,"str")==0)
	{
		if( (item->value_type==ITEM_VALUE_TYPE_STR) || (item->value_type==ITEM_VALUE_TYPE_LOG))
		{
			if(strstr(item->lastvalue_str, parameter) == NULL)
			{
				strcpy(value,"0");
			}
			else
			{
				strcpy(value,"1");
			}

		}
		else
		{
			ret = FAIL;
		}
	}
	else if(strcmp(function,"regexp")==0)
	{
		if( (item->value_type==ITEM_VALUE_TYPE_STR) || (item->value_type==ITEM_VALUE_TYPE_LOG))
		{
			if(zbx_regexp_match(item->lastvalue_str, parameter, &len) != NULL)
			{
				strcpy(value,"1");
			}
			else
			{
				strcpy(value,"0");
			}
		}
		else
		{
			ret = FAIL;
		}
	}
	else if(strcmp(function,"now")==0)
	{
		now=time(NULL);
                zbx_snprintf(value,MAX_STRING_LEN,"%d",(int)now);
	}
	else if(strcmp(function,"fuzzytime")==0)
	{
		now=time(NULL);
		fuzlow=(int)(now-atoi(parameter));
		fuzhig=(int)(now+atoi(parameter));

		if(item->lastvalue_null==1)
		{
				ret = FAIL;
		}
		else
		{
			switch (item->value_type) {
				case ITEM_VALUE_TYPE_FLOAT:
					if((item->lastvalue_dbl>=fuzlow)&&(item->lastvalue_dbl<=fuzhig))
					{
						strcpy(value,"1");
					}
					else
					{
						strcpy(value,"0");
					}
					break;
				case ITEM_VALUE_TYPE_UINT64:
					if((item->lastvalue_uint64>=fuzlow)&&(item->lastvalue_uint64<=fuzhig))
					{
						strcpy(value,"1");
					}
					else
					{
						strcpy(value,"0");
					}
					break;
				default:
					ret = FAIL;
					break;
			}
		}
	}
	else if(strcmp(function,"logseverity")==0)
	{
		ret = evaluate_LOGSEVERITY(value,item,parameter);
	}
	else if(strcmp(function,"logsource")==0)
	{
		ret = evaluate_LOGSOURCE(value,item,parameter);
	}
	else
	{
		zabbix_log( LOG_LEVEL_WARNING, "Unsupported function:%s",
			function);
		zabbix_syslog("Unsupported function:%s",
			function);
		ret = FAIL;
	}

	zabbix_log( LOG_LEVEL_DEBUG, "End of evaluate_function(result:%s)",
		value);
	return ret;
}

/******************************************************************************
 *                                                                            *
 * Function: add_value_suffix                                                 *
 *                                                                            *
 * Purpose: Add suffix for value                                              *
 *                                                                            *
 * Parameters: value - value to replacing                                     *
 *             valuemapid - index of value map                                *
 *                                                                            *
 * Return value: SUCCEED - suffix added succesfully, value contains new value *
 *               FAIL - adding failed, value contains old value               *
 *                                                                            *
 * Author: Eugene Grigorjev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
int	add_value_suffix(char *value, DB_ITEM *item)
{
	double	value_double;
	double	value_double_abs;

	char	suffix[MAX_STRING_LEN];

	zabbix_log( LOG_LEVEL_DEBUG, "In add_value_suffix() Value [%s]",
		value);
	
	/* Add suffix: 1000000 -> 1 MB */
	if(!(
		(ITEM_VALUE_TYPE_FLOAT == item->value_type) &&
		(strlen(item->units)>0))
	)	return FAIL;
		
	value_double=atof(value);
	/* Custom multiplier? */
/*
	if(item->multiplier == 1)
	{
		value_double=value_double*atof(item->formula);
	}*/

	value_double_abs=abs(value_double);

	if(value_double_abs<1024)
	{
		strscpy(suffix,"");
	}
	else if(value_double_abs<1024*1024)
	{
		strscpy(suffix,"K");
		value_double=value_double/1024;
	}
	else if(value_double_abs<1024*1024*1024)
	{
		strscpy(suffix,"M");
		value_double=value_double/(1024*1024);
	}
	else
	{
		strscpy(suffix,"G");
		value_double=value_double/(1024*1024*1024);
	}
/*		if(cmp_double((double)round(value_double), value_double) == 0) */
	if(cmp_double((int)(value_double+0.5), value_double) == 0)
	{
		zbx_snprintf(value, MAX_STRING_LEN, ZBX_FS_DBL_EXT(0) " %s%s",
			value_double,
			suffix,
			item->units);
	}
	else
	{
		zbx_snprintf(value, MAX_STRING_LEN, ZBX_FS_DBL_EXT(2) " %s%s",
			value_double,
			suffix,
			item->units);
	}
	
	zabbix_log(LOG_LEVEL_DEBUG, "Value [%s] [" ZBX_FS_DBL "] Suffix [%s] Units [%s]",
		value,
		value_double,
		suffix,
		item->units);
	return SUCCEED;
}

/******************************************************************************
 *                                                                            *
 * Function: replace_value_by_map                                             *
 *                                                                            *
 * Purpose: replace value by mapping value                                    *
 *                                                                            *
 * Parameters: value - value to replacing                                     *
 *             valuemapid - index of value map                                *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, value contains new value    *
 *               FAIL - evaluation failed, value contains old value           *
 *                                                                            *
 * Author: Eugene Grigorjev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
int	replace_value_by_map(char *value, zbx_uint64_t valuemapid)
{
	DB_RESULT	result;
	DB_ROW		row;

	char new_value[MAX_STRING_LEN];
	char sql[MAX_STRING_LEN];
	char *or_value;

	zabbix_log(LOG_LEVEL_DEBUG, "In replace_value_by_map()" );
	
	if(valuemapid == 0)	return FAIL;
	
	result = DBselect("select newvalue from mappings where valuemapid=" ZBX_FS_UI64 " and value='%s'",
			valuemapid,
			value);
	row = DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)		return FAIL;

	strcpy(new_value,row[0]);
	DBfree_result(result);

	del_zeroes(new_value);
	or_value = sql;	/* sql variarbvle used as tmp - original value */
	zbx_strlcpy(sql,value,MAX_STRING_LEN);
	
	zbx_snprintf(value, MAX_STRING_LEN, "%s (%s)",
		new_value,
		or_value);

	zabbix_log(LOG_LEVEL_DEBUG, "End replace_value_by_map(result:%s)",
		value);
	return SUCCEED;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_function2                                               *
 *                                                                            *
 * Purpose: evaluate function                                                 *
 *                                                                            *
 * Parameters: host - host the key belongs to                                 *
 *             key - item's key (for example, 'max')                          *
 *             function - function (for example, 'max')                       *
 *             parameter - parameter of the function)                         *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, value contains its value    *
 *               FAIL - evaluation failed                                     *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments: Used for evaluation of notification macros                       *
 *                                                                            *
 ******************************************************************************/
int evaluate_function2(char *value,char *host,char *key,char *function,char *parameter)
{
	DB_ITEM	item;
	DB_RESULT result;
	DB_ROW	row;

	int	res;

	zabbix_log(LOG_LEVEL_DEBUG, "In evaluate_function2(%s,%s,%s,%s)",
		host,
		key,
		function,
		parameter);

	result = DBselect("select %s where h.host='%s' and h.hostid=i.hostid and i.key_='%s' and" ZBX_COND_NODEID,
		ZBX_SQL_ITEM_SELECT,
		host,
		key,
		LOCAL_NODE("h.hostid"));

	row = DBfetch(result);

	if(!row)
	{
        	DBfree_result(result);
		zabbix_log(LOG_LEVEL_WARNING, "Query returned empty result");
		zabbix_syslog("Query returned empty result");
		return FAIL;
	}

	DBget_item_from_db(&item,row);

	res = evaluate_function(value,&item,function,parameter);

	if(replace_value_by_map(value, item.valuemapid) != SUCCEED)
	{
		add_value_suffix(value, &item);
	}

/* Cannot call DBfree_result until evaluate_FUNC */
	DBfree_result(result);

	zabbix_log(LOG_LEVEL_DEBUG, "End evaluate_function2(result:%s)",
		value);
	return res;
}
