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
  vx_status;
  (void)node;

  if (num != 3)
    return VX_ERROR_INVALID_PARAMETERS;

  for (vx_uint32 i = 0u; i < num; i++)
    {
      if (!parameters[i])
        return VX_ERROR_INVALID_REFERENCE;

      switch (num)
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
              return VX_ERROR_INVALID_VALUE;
          }
          break;

        case 1:
          {
            vx_uint32 bayer_pattern;
            if (vxQueryParameter ((vx_parameter)parameters[1],
                                  VX_PARAMETER_REF, &bayer_pattern,
                                  sizeof (bayer_pattern))
                != VX_SUCCESS)
              return VX_ERROR_INVALID_PARAMETERS;
          }
          break;

        case 2:
          {

            vx_image input = (vx_image)parameters[0];
            vx_uint32 width = 0, height = 0;
            vx_df_image format = VX_DF_IMAGE_RGB;
            if (vxQueryImage (input, VX_IMAGE_WIDTH, &width, sizeof (width))
                    != VX_SUCCESS
                || vxQueryImage (input, VX_IMAGE_HEIGHT, &height,
                                 sizeof (height) != VX_SUCCESS))
              return VX_ERROR_INVALID_PARAMETERS;
            if (vxSetMetaFormatAttribute (metas[i], VX_IMAGE_FORMAT, &format,
                                          sizeof (format) != VX_SUCCESS)
                || vxSetMetaFormatAttribute (metas[i], VX_IMAGE_WIDTH, &width,
                                             sizeof (width))
                       != VX_SUCCESS
                || vxSetMetaFormatAttribute (metas[i], VX_IMAGE_HEIGHT,
                                             &height,
                                             sizeof (height) != VX_SUCCESS))
              return VX_ERROR_INVALID_VALUE;
          }
        }
      return VX_SUCCESS;
    }
}

vx_status
vxDemosaic (vx_image src, vx_image dst, vx_uint32 pattern)
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

  status = vxGetValidRegionImage (src, &rect);

  status
      = vxMapImagePatch (src, &rect, 0, &src_map_id, &src_addr, &src_base_ptr,
                         VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

  status |= vxMapImagePatch (dst, &rect, 0, &dst_map_id, &dst_addr_r,
                             &dst_base_ptr_r, VX_WRITE_ONLY,
                             VX_MEMORY_TYPE_HOST, 0);

  status |= vxMapImagePatch (dst, &rect, 1, &dst_map_id, &dst_addr_g,
                             &dst_base_ptr_g, VX_WRITE_ONLY,
                             VX_MEMORY_TYPE_HOST, 0);

  status |= vxMapImagePatch (dst, &rect, 2, &dst_map_id, &dst_addr_b,
                             &dst_base_ptr_b, VX_WRITE_ONLY,
                             VX_MEMORY_TYPE_HOST, 0);

  /* 2d addressing option */
  for (int y = 0; y < src_addr.dim_y; y += src_addr.step_y)
    {
      for (int x = 0; x < src_addr.dim_x; x += src_addr.step_x)
        {
          vx_uint16 *byr
              = vxFormatImagePatchAddress2d (src_base_ptr, x, y, &src_addr);
          vx_uint8 *r = vxFormatImagePatchAddress2d (dst_base_ptr_r, x, y,
                                                     &dst_addr_r);
          vx_uint8 *g = vxFormatImagePatchAddress2d (dst_base_ptr_r, x, y,
                                                     &dst_addr_g);
          vx_uint8 *b = vxFormatImagePatchAddress2d (dst_base_ptr_r, x, y,
                                                     &dst_addr_b);

          r = 0xff;
          g = 0;
          b = 0;
        }
    }

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
  if (num == 3)
    {
      vx_image input = (vx_image)parameters[0];
      vx_uint32 bayer_pattern = (vx_uint32)parameters[1];
      vx_image output = (vx_image)parameters[2];
      status = vxDemosaic (input, output, bayer_pattern);
    }
  return status;
}

vx_node
vxDemosaicNode (vx_graph graph, vx_image input, vx_uint32 bayer_pattern,
                vx_image output)
{

  vx_reference params[] = {
    (vx_reference)input,
    (vx_reference)bayer_pattern,
    (vx_reference)output,
  };

  return vxCreateNodeByStructure (graph, VX_KERNEL_KHR_DEMOSAIC, params,
                                  dimof (params));
}

static vx_param_description_t demosaic_kernel_params[] = {
  { VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED },
  { VX_INPUT, VX_TYPE_UINT32, VX_PARAMETER_STATE_REQUIRED },
  { VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED },
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
