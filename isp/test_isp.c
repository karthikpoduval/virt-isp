#include <VX/vx.h>
#include "vx_lib_isp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void vxLogCb(
            vx_context context,
            vx_reference ref,
            vx_status status,
            const vx_char string[])
{
    size_t len = strlen(string);
    if (len > 0) {
        printf("%s", string);
        if (string[len - 1] != '\n')
            printf("\n");
        fflush(stdout);
    }
	//printf("%p:%d:%s\n", context, status, string);
}

int main()
{
vx_status status;
vx_context context = vxCreateContext();
vxRegisterLogCallback(context, vxLogCb,  vx_false_e);
status = vxLoadKernels(context, "openvx-isp");
if (status != VX_SUCCESS)
{
	printf("Load kernel FAIL status=%d\n", status);
	exit(-1);
}
vx_image images[] = {
        vxCreateImage(context, 640, 480, VX_DF_IMAGE_U16),
        vxCreateImage(context, 640, 480, VX_DF_IMAGE_RGB),
};

vx_graph graph = vxCreateGraph(context);
vx_uint32 bayerPattern = 0;
vxDemosaicNode(graph, images[0], images[1]);

status = vxVerifyGraph(graph);
if (status == VX_SUCCESS)
{
    printf("verify graph success\n");
    status = vxProcessGraph(graph);
}
else
{
    printf("verify graph FAIL status=%d\n", status);

}
vxReleaseContext(&context); /* this will release everything */
printf("DONE\n");
}
