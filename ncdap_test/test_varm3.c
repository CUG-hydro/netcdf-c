/* varm_test */
/* acm 9/15/09 */
/* ansley.b.manke@noaa.gov */
/* test nc_get_varm_float with calls similar to Ferret calls */
/* local file is correct results with stride: every second value */
/* remote url; incorrect results with stride 

linked with:
 cc varm_test.c -g -o varm_test /home/nstout/ansley/local/lib/libnetcdf.a -L/usr/lib64 -lc -lm -lcurl

netcdf.a from the daily snapshot
netcdf-4.1-beta2-snapshot2009091100

TODO: Note that this test uses thredds server
*/

/* This particular test seems to occasionally expose a server error*/


#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "netcdf.h"
#include "ncdispatch.h"
#include "nctestserver.h"

#undef STANDALONE

#undef DEBUG

#define TESTPATH "/dodsC/testdods/coads_climatology.nc"
#define VAR "SST"

static float expected_stride1[12] = {
29.430857f,
29.403780f,
29.325428f,
29.578333f,
29.660833f,
29.378437f,
29.151943f,
29.109715f,
29.114864f,
29.550285f,
29.542500f,
29.500286f
};

static float expected_stride2[6] = {
29.430857f,
29.325428f,
29.660833f,
29.151943f,
29.114864f,
29.542500f
};

static float expected_stride3[3] = {
29.430857f,
29.378437f,
29.542500f
};

void
check(int status, char* file, int line)
{
    if(status == 0) return;
    fprintf(stderr,"error: %s at %s:%d\n",nc_strerror(status),file,line);
    exit(1);
}

int
main()
{

    int ncid;
    int varid;
    int i,fail;
    int err;
    size_t start[5], count[5];
    ptrdiff_t stride[5], imap[5];
    int idim;
    float dat[20];
    char url[4096];
    char* svc;
#ifdef STANDALONE
    int ndim;
#endif

#ifdef DEBUG
    oc_loginit();
    oc_setlogging(1);
    oc_logopen(NULL);
#endif

    /* Find Test Server */
    svc = nc_findtestserver("thredds",REMOTETESTSERVERS);

    if(svc == NULL) {
        fprintf(stderr,"WARNING: Cannot locate test server\n");
	exit(0);
    }
    strncpy(url,svc,sizeof(url));
    strlcat(url,TESTPATH,sizeof(url));
    printf("*** Test: varm on URL: %s\n",url);
    free(svc);

    check(err = nc_open(url, NC_NOWRITE, &ncid),__FILE__,__LINE__);
    check(err = nc_inq_varid(ncid, VAR, &varid),__FILE__,__LINE__);
    for (idim=0; idim<4; idim++) {
        start[idim] = 0;
        count[idim] = 1;
        stride[idim] = 1;
        imap[idim] = 1;
    }
#ifdef STANDALONE
    ndim=3;
#endif

    printf("*** Testing: stride case 1\n");
    start[1] = 44;
    start[2] = 66;
    count[0] = 12;

#ifdef STANDALONE
    printf("start = ");
    for(i=0;i<ndim;i++) printf(" %d",(int)start[i]);
    printf("\n");
    printf("count = ");
    for(i=0;i<ndim;i++) printf(" %d",(int)count[i]);
    printf("\n");
    printf("stride = ");
    for(i=0;i<ndim;i++) printf(" %d",(int)stride[i]);
    printf("\n");
    printf("map = ");
    for(i=0;i<ndim;i++) printf(" %d",(int)imap[i]);
    printf("\n");

    err = nc_get_vars_float (ncid, varid, start, count, stride,
			     (float*) dat);
    printf("vars: %s =",VAR);
    for(i=0;i<12;i++) printf(" %f",dat[i]);
    printf("\n");
#endif

    check(err = nc_get_varm_float (ncid, varid, start, count, stride, imap,(float*) dat),__FILE__,__LINE__);
    /*    check(err = nc_get_vara_float (ncid, varid, start, count, (float*) dat),__FILE__,__LINE__); */

#ifdef STANDALONE
    printf("varm: %s =",VAR);
    for(i=0;i<12;i++) printf(" %f",dat[i]);
    printf("\n");
#endif
    fail=0;
    for(i=0;i<12;i++) {
	float delta = (dat[i] - expected_stride1[i]);
	if(delta > 0.0005 || delta < -0.0005) {
	    fprintf(stderr,"*** Failure: unexpected value: delta=%g dat[%d]=%g expected[%d]=%g\n",
		    delta, i, dat[i], i, expected_stride1[i]);
	     fail = 1;
	}
    }
    printf("*** %s: stride case 1\n",(fail?"Fail":"Pass"));

    printf("*** Testing: stride case 2\n");
    /* case with strides #1 where len % stride == 0 */
    start[1] = 44;
    start[2] = 66;
    count[0] =  6;
    stride[0] = 2;

#ifdef STANDALONE
    printf("start = ");
    for(i=0;i<ndim;i++) printf(" %d",(int)start[i]);
    printf("\n");
    printf("count = ");
    for(i=0;i<ndim;i++) printf(" %d",(int)count[i]);
    printf("\n");
    printf("stride = ");
    for(i=0;i<ndim;i++) printf(" %d",(int)stride[i]);
    printf("\n");
    printf("map = ");
    for(i=0;i<ndim;i++) printf(" %d",(int)imap[i]);
    printf("\n");

    check(err = nc_get_vars_float(ncid, varid, start, count, stride, 
                             (float*) dat),__FILE__,__LINE__);
    printf("strided.vars: %s =",VAR);
    for(i=0;i<6;i++) printf(" %f",dat[i]);
    printf("\n");
#endif
    check(err = nc_get_varm_float(ncid, varid, start, count, stride, imap,
                             (float*) dat),__FILE__,__LINE__);
#ifdef STANDALONE
    printf("strided.varm: %s =",VAR);
    for(i=0;i<6;i++) printf(" %f",dat[i]);
    printf("\n");
#endif
    fail=0;
    for(i=0;i<6;i++) {
	float delta = (dat[i] - expected_stride2[i]);
	if(delta > 0.0005 || delta < -0.0005) {
	    fprintf(stderr,"*** Failure: unexpected value: delta=%g dat[%d]=%g expected[%d]=%g\n",
		    delta, i, dat[i], i, expected_stride2[i]);
	    fail=1;
	}
    }
    printf("*** %s: stride case 2\n",(fail?"Fail":"Pass"));

    /* case with strides #2: len % stride != 0 */
    printf("*** Testing: stride case 3\n");
    start[1] = 44;
    start[2] = 66;
    count[0] =  3;
    stride[0] = 5;

#ifdef STANDALONE
    printf("start = ");
    for(i=0;i<ndim;i++) printf(" %d",(int)start[i]);
    printf("\n");
    printf("count = ");
    for(i=0;i<ndim;i++) printf(" %d",(int)count[i]);
    printf("\n");
    printf("stride = ");
    for(i=0;i<ndim;i++) printf(" %d",(int)stride[i]);
    printf("\n");
    printf("map = ");
    for(i=0;i<ndim;i++) printf(" %d",(int)imap[i]);
    printf("\n");

    check(err = nc_get_vars_float(ncid, varid, start, count, stride, 
                             (float*) dat),__FILE__,__LINE__);
    printf("strided.vars: %s =",VAR);
    for(i=0;i<3;i++) printf(" %f",dat[i]);
    printf("\n");
#endif
    check(err = nc_get_varm_float(ncid, varid, start, count, stride, imap,
                             (float*) dat),__FILE__,__LINE__);
#ifdef STANDALONE
    printf("strided.varm: %s =",VAR);
    for(i=0;i<3;i++) printf(" %f",dat[i]);
    printf("\n");
#endif
    fail=0;
    for(i=0;i<3;i++) {
	float delta = (dat[i] - expected_stride3[i]);
	if(delta > 0.0005 || delta < -0.0005) {
	    fprintf(stderr,"*** Failure: stride case 2: unexpected value: delta=%g dat[%d]=%g expected[%d]=%g\n",
		    delta, i, dat[i], i, expected_stride3[i]);
	    fail=1;
	}
    }
    printf("*** %s: stride case 3\n",(fail?"Fail":"Pass"));

    nc_close(ncid);
    return fail;

}
