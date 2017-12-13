#ifndef DBL_LNK_INCLUDED
#define DBL_LNK_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
#define DBL_LNK_SUCCESS 0
#define DBL_LNK_FAILURE 1

/************************************************************************/

typedef struct dbl_lnk
  {
  struct dbl_lnk *next;
  struct dbl_lnk *prev;
  } DBL_LNK;

/************************************************************************/

int	 dblLnkAddFirst (DBL_LNK **listHead, DBL_LNK *newNode);
int	 dblLnkAddLast  (DBL_LNK **listHead, DBL_LNK *newNode);	

DBL_LNK *dblLnkUnlinkFirst (DBL_LNK **listHead);		
DBL_LNK *dblLnkUnlinkLast  (DBL_LNK **listHead);


#ifdef __cplusplus
}
#endif

#endif
