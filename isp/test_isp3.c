#include <VX/vx.h>
#include "vx_lib_isp.h"
#include <VX/vx_lib_debug.h>
#include <VX/vx_helper.h>
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
    vx_status status = VX_FAILURE;
    vx_context context = vxCreateContext();

    vxRegisterLogCallback(context, vxLogCb, vx_false_e);
    if (vxGetStatus((vx_reference)context) == VX_SUCCESS)
    {
        vx_uint32 i = 0, diffs = 0u;
        vx_uint32 w = 640, h = 480;
	vx_char ext_kernels[][VX_MAX_FILE_NAME] = {
		"openvx-isp",
		"openvx-debug"
	};
        vx_image images[] = {
            vxCreateImage(context, w, h, VX_DF_IMAGE_U16),    /* 0: u16 */
            vxCreateImage(context, w, h, VX_DF_IMAGE_RGB),    /* 1: rgb */
        };
        status = VX_SUCCESS;
        for (i = 0; i < dimof(images); i++)
        {
            if (images[i] == 0)
            {
                status = VX_ERROR_NOT_SUFFICIENT;
            }
        }
        for (i = 0; i < dimof(ext_kernels); i++)
	{
        	status |= vxLoadKernels(context, ext_kernels[i]);
		printf("load kernel=%s\n", ext_kernels[i]);
        	//status |= vxLoadKernels(context, "openvx-isp");
	}
        if (status == VX_SUCCESS)
        {
            vx_graph graph = vxCreateGraph(context);
            if (vxGetStatus((vx_reference)graph) == VX_SUCCESS)
            {
                vx_node nodes[] = {
	            vxDemosaicNode(graph, images[0], images[1]),
                    //vxFWriteImageNode(graph, images[1], "demosaic.rgb"),
                };
                //CHECK_ALL_ITEMS(nodes, i, status, exit);
                if (status == VX_SUCCESS)
                {
                    status = vxVerifyGraph(graph);
                    if (status == VX_SUCCESS)
                    {
                        status = vxProcessGraph(graph);
			printf("Graph run successfully\n");
                    }
		    else
		    {
			printf("graph processing failed");
		    }
                    for (i = 0; i < dimof(nodes); i++)
                    {
                        vxReleaseNode(&nodes[i]);
                    }
                }
                vxReleaseGraph(&graph);
            }
            vxuFWriteImage(context, images[1], "demosaic.rgb");
           // status |= vxUnloadKernels(context, "openvx-isp");
            for (i = 0; i < dimof(ext_kernels); i++)
	    {
        	status |= vxUnloadKernels(context, ext_kernels[i]);
		printf("unload kernel=%s\n", ext_kernels[i]);
	    }
        }
        for (i = 0; i < dimof(images); i++)
        {
            vxReleaseImage(&images[i]);
        }
        vxReleaseContext(&context);
    }

}
