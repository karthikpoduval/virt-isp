/*
 * Copyright (c) 2023 Karthik Poduval <karthik.poduval@gmail.com>
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License, Version 2.0
 * which accompanies this distribution, and is available at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "vx_lib_isp.h"
#include <VX/vx.h>
#include <VX/vx_helper.h>
#include <stdio.h>

#define TOP_LEFT(x) x[0][0]
#define TOP_CENTER(x) x[1][0]
#define TOP_RIGHT(x) x[2][0]
#define MID_LEFT(x) x[0][1]
#define MID_CENTER(x) x[1][1]
#define MID_RIGHT(x) x[2][1]
#define BOTTOM_LEFT(x) x[0][0]
#define BOTTOM_CENTER(x) x[1][0]
#define BOTTOM_RIGHT(x) x[2][0]

/*! \brief demosaic parameter validator.
 * \param [in] node The handle to the node.
 * \param [in] parameters The array of parameters to be validated.
 * \param [in] num Number of parameters to be validated.
 * \param [out] metas The array of metadata used to check output parameters
 * only. \return A \ref vx_status_e enumeration. \ingroup group_example_kernel
 */
vx_status VX_CALLBACK
vxDemosaicValidator (vx_node node, const vx_reference parameters[],
                     vx_uint32 num, vx_meta_format metas[])
{
  vxAddLogEntry((vx_reference)node, VX_SUCCESS, "%s \n", __func__);
  printf("%s num=%d\n", __func__, num);
  vx_status;
  (void)node;
  if (num != 2)
    return VX_ERROR_INVALID_PARAMETERS;
	
  for (vx_uint32 i = 0u; i < num; i++)
    {
      if (!parameters[i])
        return VX_ERROR_INVALID_REFERENCE;

      switch (i)
        {
        case 0:
          {
            vx_df_image input_image = 0;

            if (vxQueryImage ((vx_image)parameters[i], VX_IMAGE_FORMAT,
                              &input_image, sizeof (input_image))
                != VX_SUCCESS)
              return VX_ERROR_INVALID_PARAMETERS;

            /* RAW BAYER Needs to be upto 16 bit */
            if (input_image != VX_DF_IMAGE_U16)
	    {
  	      vxAddLogEntry((vx_reference)node, VX_SUCCESS, "Not VX_DF_IMAGE_U16");
  	      printf("Not VX_DF_IMAGE_U16");
              return VX_ERROR_INVALID_VALUE;
	    }
          }
          break;
        case 1:
          {
	   
            vx_image input = (vx_image)parameters[0];
            vx_uint32 width = 0, height = 0;
            vx_df_image format = VX_DF_IMAGE_RGB;
	    vx_status s = vxQueryImage(input, VX_IMAGE_WIDTH, &width, sizeof (width));
	    printf("status %d\n", s);
	    vx_status s2 = vxQueryImage(input, VX_IMAGE_HEIGHT, &height, sizeof (height));
	    printf("status %d\n", s2);
            //if ((vxQueryImage(input, VX_IMAGE_WIDTH, &width, sizeof (width)) != VX_SUCCESS)
            //    || (vxQueryImage (input, VX_IMAGE_HEIGHT, &height, sizeof (height) != VX_SUCCESS)))
	     if(s != VX_SUCCESS || s2 != VX_SUCCESS)
	    {

  	      printf("unable to query input image\n");
              return VX_ERROR_INVALID_PARAMETERS;
	    }
	    else {
	    	printf("width=%d height=%d\n", width, height);
	    }
	    vx_status s3 = vxSetMetaFormatAttribute (metas[i], VX_IMAGE_FORMAT, &format, sizeof (format)); 
	    vx_status s4 = vxSetMetaFormatAttribute (metas[i], VX_IMAGE_WIDTH, &width, sizeof (width)); 
	    vx_status s5 = vxSetMetaFormatAttribute (metas[i], VX_IMAGE_HEIGHT, &height, sizeof (height)); 
#if 0
            if (vxSetMetaFormatAttribute (metas[i], VX_IMAGE_FORMAT, &format,
                                          sizeof (format) != VX_SUCCESS)
                || vxSetMetaFormatAttribute (metas[i], VX_IMAGE_WIDTH, &width,
                                             sizeof (width))
                       != VX_SUCCESS
                || vxSetMetaFormatAttribute (metas[i], VX_IMAGE_HEIGHT,
                                             &height,
                                             sizeof (height) != VX_SUCCESS))
#endif
	     if(s3 || s4 || s5)
	     {
  	      printf("unable to set meta\n");
              return VX_ERROR_INVALID_VALUE;
	     }
          }
        }
    }
    return VX_SUCCESS;
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


vx_status
vxDemosaic (vx_image src, vx_image dst)
{
  vx_status status = VX_SUCCESS;
  void *src_base = NULL;
  void *dst_base = NULL;
  vx_df_image format = 0;
  vx_uint32 plane = 0;
  vx_imagepatch_addressing_t src_addr, dst_addr_r, dst_addr_g, dst_addr_b;
  vx_map_id src_map_id, dst_map_id;
  void *src_base_ptr = NULL;
  void *dst_base_ptr_r = NULL;
  void *dst_base_ptr_g = NULL;
  void *dst_base_ptr_b = NULL;
  vx_rectangle_t rect;

  printf("############################\n");
  status = vxGetValidRegionImage (src, &rect);
  if(status == VX_SUCCESS)
  {
  status = vxMapImagePatch (src, &rect, 0, &src_map_id, &src_addr, &src_base_ptr,
                         VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

  status |= vxMapImagePatch (dst, &rect, 0, &dst_map_id, &dst_addr_r,
                             &dst_base_ptr_r, VX_WRITE_ONLY,
                             VX_MEMORY_TYPE_HOST, 0);
#if 0
  status |= vxMapImagePatch (dst, &rect, 1, &dst_map_id, &dst_addr_g,
                             &dst_base_ptr_g, VX_WRITE_ONLY,
                             VX_MEMORY_TYPE_HOST, 0);

  status |= vxMapImagePatch (dst, &rect, 2, &dst_map_id, &dst_addr_b,
                             &dst_base_ptr_b, VX_WRITE_ONLY,
                             VX_MEMORY_TYPE_HOST, 0);
#endif
  } else {
	  printf("Unable to access image rect\n");
  }
  /* 2d addressing option */
  for (int y = 0; y < dst_addr_r.dim_y; y += dst_addr_r.step_y)
    {
      for (int x = 0; x < dst_addr_r.dim_x; x += dst_addr_r.step_x)
        {
         // vx_uint16 *byr = vxFormatImagePatchAddress2d (src_base_ptr, x, y, &src_addr);
	 // printf("%x\n", *byr);
          vx_uint8 *rgb = vxFormatImagePatchAddress2d (dst_base_ptr_r, x, y,
                                                     &dst_addr_r);
	  //printf("x=%d y=%d ptr=%p\n", x, y, rgb);
	  vx_uint8 *r = &rgb[0];
	  vx_uint8 *g = &rgb[1];
	  vx_uint8 *b = &rgb[2];
#if 0
     	  vx_uint8 *g = vxFormatImagePatchAddress2d (dst_base_ptr_g, x, y,
                                                     &dst_addr_g);
          vx_uint8 *b = vxFormatImagePatchAddress2d (dst_base_ptr_b, x, y,
                                                     &dst_addr_b);
	  //printf("%p\n", r); 
          r[0] = 0xff;
          r[1] = 0xff;
          r[2] = 0xff;
          //g = 0;
          //b = 0;
#endif
	/* use 3x3 patch to interpolate values for each color channel */
	vx_uint16 window[3][3] = {0,};
	vx_uint16 *ptr;

	//ptr = vxFormatImagePatchAddress2d(src_base_ptr, x, y, &src_addr);
	/* fill the window with pixels and do boundary mirroring if necessary */
	/* the following steaps are only possible for x > 3 and y > 3 */
	if((x>3) && (y>3) && x<=(src_addr.dim_x-3) && y<=(src_addr.dim_y-3))
	{
		window[0][0] = *((vx_uint16*)vxFormatImagePatchAddress2d(src_base_ptr, x-3, y-3, &src_addr));
		window[0][1] = *((vx_uint16*)vxFormatImagePatchAddress2d(src_base_ptr, x-2, y-3, &src_addr));
		window[0][2] = *((vx_uint16*)vxFormatImagePatchAddress2d(src_base_ptr, x-1, y-3, &src_addr));
		window[1][0] = *((vx_uint16*)vxFormatImagePatchAddress2d(src_base_ptr, x-3, y-2, &src_addr));
		window[1][1] = *((vx_uint16*)vxFormatImagePatchAddress2d(src_base_ptr, x-2, y-2, &src_addr));
		window[1][2] = *((vx_uint16*)vxFormatImagePatchAddress2d(src_base_ptr, x-1, y-2, &src_addr));
		window[2][0] = *((vx_uint16*)vxFormatImagePatchAddress2d(src_base_ptr, x-3, y-1, &src_addr));
		window[2][1] = *((vx_uint16*)vxFormatImagePatchAddress2d(src_base_ptr, x-2, y-1, &src_addr));
		window[2][2] = *((vx_uint16*)vxFormatImagePatchAddress2d(src_base_ptr, x-1, y-1, &src_addr));
	}
	else
	{
		//window[1][1] = *((vx_uint16*)vxFormatImagePatchAddress2d(src_base_ptr, x, y, &src_addr));
	}

	/* find in current pixel is B,Gr,Gb or R */
	int bayerPixel = getPixelFromBayerPattern(VX_DEMOSAIC_PATTERN_RGGB , x, y);
	switch(bayerPixel)
	{
		case VX_DEMOSAIC_PIXEL_B:
			{
				*r = ((TOP_LEFT(window) + TOP_RIGHT(window) + BOTTOM_LEFT(window) + BOTTOM_RIGHT(window))/4)>>6;
				*g = ((TOP_CENTER(window) + MID_LEFT(window) + MID_RIGHT(window) + BOTTOM_CENTER(window))/4)>>6;
				*b = MID_CENTER(window)>>6;
			}
			break;
		case VX_DEMOSAIC_PIXEL_Gr:
			{
				*r = ((MID_LEFT(window) + MID_RIGHT(window))/2)>>6;
				*g = MID_CENTER(window)>>6;
				*b = ((TOP_CENTER(window) + BOTTOM_CENTER(window))/2)>>6;
			}
			break;
		case VX_DEMOSAIC_PIXEL_Gb:
			{
				*r = ((TOP_CENTER(window) + BOTTOM_CENTER(window))/2)>>6;
				*g = MID_CENTER(window)>>6;
				*b = ((MID_LEFT(window) + MID_RIGHT(window))/2)>>6;
			}
			break;
		case VX_DEMOSAIC_PIXEL_R:
			{
				*r = MID_CENTER(window)>>6;
				*g = ((TOP_CENTER(window) + MID_LEFT(window) + MID_RIGHT(window) + BOTTOM_CENTER(window))/4)>>6;
				*b = ((TOP_LEFT(window) + TOP_RIGHT(window) + BOTTOM_LEFT(window) + BOTTOM_RIGHT(window))/4)>>6;
			}
			break;

	}
	//*r = 0;
	//*g = 0;
	//*b = 0xff;
        }
    }
  status = vxUnmapImagePatch(src, src_map_id);
  status = vxUnmapImagePatch(dst, dst_map_id);

  return status;
}

/*!
 * \brief The private kernel function for TestModule.
 * \note This is not called directly by users.
 * \param [in] node The handle to the node this kernel is instanced into.
 * \param [in] parameters The array of \ref vx_reference references.
 * \param [in] num The number of parameters in the array.
 * functions.
 * \return A \ref vx_status_e enumeration.
 * \retval VX_SUCCESS Successful return.
 * \retval VX_ERROR_INVALID_PARAMETER The input or output image were
 * of the incorrect dimensions.
 * \ingroup group_isp_kernel
 */
vx_status VX_CALLBACK
vxDemosaicKernel (vx_node node, const vx_reference *parameters, vx_uint32 num)
{
  vx_status status = VX_ERROR_INVALID_PARAMETERS;
  if (num == 2)
    {
      vx_image input = (vx_image)parameters[0];
      vx_image output = (vx_image)parameters[1];
      status = vxDemosaic (input, output);
    }
  return status;
}

vx_node
//vxDemosaicNode (vx_graph graph, vx_image input, vx_uint32 bayer_pattern,
vxDemosaicNode (vx_graph graph, vx_image input,
                vx_image output)
{
  printf("%s\n", __func__);
  vx_reference params[] = {
    (vx_reference)input,
    (vx_reference)output,
  };

  return vxCreateNodeByStructure (graph, VX_KERNEL_KHR_DEMOSAIC, params,
                                  dimof (params));
}

static vx_param_description_t demosaic_kernel_params[] = {
  { VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED },
  { VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED },
};

vx_kernel_description_t demosaic_kernel = {
  VX_KERNEL_KHR_DEMOSAIC,
  VX_KERNEL_NAME_KHR_ISP_DEMOSAIC,
  vxDemosaicKernel,
  demosaic_kernel_params,
  dimof (demosaic_kernel_params),
  vxDemosaicValidator,
  NULL,
  NULL,
  NULL,
  NULL,
};
