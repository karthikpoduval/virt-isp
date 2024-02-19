#include <VX/vx.h>
#include <stdio.h>

int main()
{

vx_status status;	
vx_context context = vxCreateContext();
vx_image images[] = {
        vxCreateImage(context, 640, 480, VX_DF_IMAGE_UYVY),
        vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16),
        vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8),
};
vx_graph graph = vxCreateGraph(context);
vx_image virts[] = {
        vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT),
        vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT),
        vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT),
        vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT),
};

vxChannelExtractNode(graph, images[0], VX_CHANNEL_Y, virts[0]),
vxGaussian3x3Node(graph, virts[0], virts[1]),
vxSobel3x3Node(graph, virts[1], virts[2], virts[3]),
vxMagnitudeNode(graph, virts[2], virts[3], images[1]),
vxPhaseNode(graph, virts[2], virts[3], images[2]),

status = vxVerifyGraph(graph);
if (status == VX_SUCCESS)
{
	printf("SUCCESS \n");
    status = vxProcessGraph(graph);
}
vxReleaseContext(&context); /* this will release everything */

	printf("SUCCESS\ n");
}
