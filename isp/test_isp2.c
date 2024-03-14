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

    vxRegisterLogCallback(context, vxLogCb,  vx_false_e);
    if (vxGetStatus((vx_reference)context) == VX_SUCCESS)
    {
        vx_uint32 i = 0, diffs = 0u;
        vx_uint32 w = 640, h = 480;
        vx_image images[] = {
            vxCreateImage(context, w, h, VX_DF_IMAGE_RGB),    /* 0: rgb */
            vxCreateImage(context, w, h, VX_DF_IMAGE_U8),     /* 1: r */
            vxCreateImage(context, w, h, VX_DF_IMAGE_U8),     /* 2: g */
            vxCreateImage(context, w, h, VX_DF_IMAGE_U8),     /* 3: b */
            vxCreateImage(context, w, h, VX_DF_IMAGE_RGB),    /* 4: rgb */
        };
        status = VX_SUCCESS;
        for (i = 0; i < dimof(images); i++)
        {
            if (images[i] == 0)
            {
                status = VX_ERROR_NOT_SUFFICIENT;
            }
        }
        status |= vxLoadKernels(context, "openvx-debug");
        if (status == VX_SUCCESS)
        {
            vx_graph graph = vxCreateGraph(context);
            if (vxGetStatus((vx_reference)graph) == VX_SUCCESS)
            {
                vx_node nodes[] = {
                    vxFReadImageNode(graph, "colorbars_640x480_I444.rgb", images[0]),
                    vxChannelExtractNode(graph, images[0], VX_CHANNEL_R, images[1]),
                    vxChannelExtractNode(graph, images[0], VX_CHANNEL_G, images[2]),
                    vxChannelExtractNode(graph, images[0], VX_CHANNEL_B, images[3]),
                    vxFWriteImageNode(graph, images[1], "or_640x480_P400.bw"),
                    vxFWriteImageNode(graph, images[2], "og_640x480_P400.bw"),
                    vxFWriteImageNode(graph, images[3], "ob_640x480_P400.bw"),
                    vxChannelCombineNode(graph, images[1], images[2], images[3], 0, images[4]),
                    vxFWriteImageNode(graph, images[4], "ocolorbars2_640x480_I444.rgb"),
                };
                //CHECK_ALL_ITEMS(nodes, i, status, exit);
                if (status == VX_SUCCESS)
                {
                    status = vxVerifyGraph(graph);
                    if (status == VX_SUCCESS)
                    {
                        status = vxProcessGraph(graph);
                    }
                    if (status == VX_SUCCESS)
                    {
                        // make sure images[0] == images[4]
                        status = vxuCompareImages(context, images[0], images[4], &diffs);
                        printf("Found %u differences between images (status=%d)\n", diffs, status);
                    }
                    for (i = 0; i < dimof(nodes); i++)
                    {
                        vxReleaseNode(&nodes[i]);
                    }
                }
                vxReleaseGraph(&graph);
            }
            status |= vxUnloadKernels(context, "openvx-debug");
            status |= vxUnloadKernels(context, "openvx-isp");
        }
        for (i = 0; i < dimof(images); i++)
        {
            vxReleaseImage(&images[i]);
        }
        vxReleaseContext(&context);
    }

}
