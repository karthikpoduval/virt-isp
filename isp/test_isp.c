#include <VX/vx.h>
#include "vx_lib_isp.h"
#include <VX/vx_lib_debug.h>
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

int getPixelFromBayerPattern(int pattern, int x, int y)
{
	int patternBGGR[2][2] = { {VX_DEMOSAIC_PIXEL_B, VX_DEMOSAIC_PIXEL_Gr}, {VX_DEMOSAIC_PIXEL_Gb, VX_DEMOSAIC_PIXEL_R}};
	int patternGRBG[2][2] = { {VX_DEMOSAIC_PIXEL_Gr, VX_DEMOSAIC_PIXEL_R}, {VX_DEMOSAIC_PIXEL_B, VX_DEMOSAIC_PIXEL_Gb}};
	int patternGBRG[2][2] = { {VX_DEMOSAIC_PIXEL_Gb, VX_DEMOSAIC_PIXEL_B}, {VX_DEMOSAIC_PIXEL_R, VX_DEMOSAIC_PIXEL_Gr}};
	int patternRGGB[2][2] = { {VX_DEMOSAIC_PIXEL_R, VX_DEMOSAIC_PIXEL_Gr}, {VX_DEMOSAIC_PIXEL_Gb, VX_DEMOSAIC_PIXEL_B}};
	switch(pattern)
	{

		case VX_DEMOSAIC_PATTERN_BGGR:
			return patternBGGR[x%2][y%2];
		case VX_DEMOSAIC_PATTERN_GRBG:
			return patternGRBG[x%2][y%2];
		case VX_DEMOSAIC_PATTERN_GBRG:
			return patternGBRG[x%2][y%2];
		case VX_DEMOSAIC_PATTERN_RGGB:
			return patternRGGB[x%2][y%2];
	}

}

vx_status fillTestPattern(vx_image bayerImg)
{

  vx_imagepatch_addressing_t addr;
  vx_map_id map_id;
  vx_status status = VX_SUCCESS;
  void *base_ptr = NULL;
  status = vxGetValidRegionImage (bayerImg, &rect);

  status = vxMapImagePatch(bayerImg, &rect, plane, &map_id,
                            &addr, &base_ptr,
                            VX_WRITE, VX_MEMORY_TYPE_HOST, 0);

     
    /* 2d addressing option */
    for (y = 0; y < addr.dim_y; y+=addr.step_y) {
        for (x = 0; x < addr.dim_x; x+=addr.step_x) {
            vx_uint8 *ptr2 = vxFormatImagePatchAddress2d(base_ptr,
                                                         x, y, &addr);
            int pixel = getPixelFromBayerPattern(x, y);

	    switch(pixel)
	    {
		    case VX_DEMOSAIC_PIXEL_B:

			    break;
		    case VX_DEMOSAIC_PIXEL_Gb:

			    break;
		    case VX_DEMOSAIC_PIXEL_Gr:

			    break;
		    case VX_DEMOSAIC_PIXEL_R:

			    break;
	    }
        }
    }

    status = vxUnmapImagePatch(bayerImg, map_id);
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
#if 1
status = vxLoadKernels(context, "openvx-debug");
if (status != VX_SUCCESS)
{
	printf("Load kernel FAIL status=%d\n", status);
	exit(-1);
}
#endif
vx_image images[] = {
        vxCreateImage(context, 640, 480, VX_DF_IMAGE_U16),
        vxCreateImage(context, 640, 480, VX_DF_IMAGE_RGB),
};
vx_graph graph = vxCreateGraph(context);
//vx_uint32 bayerPattern = 0;
vx_node nodes[] = {
	vxDemosaicNode(graph, images[0], images[1]),
};
//vxDemosaicNode(graph, images[0], images[1]);
vx_char rname[VX_MAX_FILE_NAME] = "demosaic.rgb";
//vxFWriteImageNode(graph, images[1], rname);
status = vxVerifyGraph(graph);
fillTestPattern(images[0]);
if (status == VX_SUCCESS)
{
    printf("verify graph success\n");
    status = vxProcessGraph(graph);
}
else
{
    printf("verify graph FAIL status=%d\n", status);

}
printf("graph success status=%d\n", status);

vxuFWriteImage(context, images[1], rname);
//vxMapImagePatch (src, &rect, 0, &src_map_id, &src_addr, &src_base_ptr,
//                         VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);


vxReleaseContext(&context); /* this will release everything */
printf("DONE\n");
}
