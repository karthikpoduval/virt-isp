/*
 * Copyright (c) 2023 Karthik Poduval <karthik.poduval@gmail.com> 
 *
 * All rights reserved. This program and the accompanying materials 
 * are made available under the terms of the Apache License, Version 2.0 
 * which accompanies this distribution, and is available at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef _OPENVX_EXT_ISP_H_
#define _OPENVX_EXT_ISP_H_

#include <VX/vx.h>

#define VX_KERNEL_NAME_KHR_ISP_DEMOSAIC "org.virt.isp.demosaic"
#define VX_LIBRARY_ISP (0x3) // assigned from Khronos, vendors control their own

/*! \brief The list of ISP Kernels.
 * \ingroup group_xyz_ext
 */
enum vx_kernel_isp_e {
    /*! \brief The Example User Defined Kernel */
    VX_KERNEL_KHR_DEMOSAIC = VX_KERNEL_BASE(VX_ID_DEFAULT, VX_LIBRARY_ISP) + 0x0,
    // up to 0xFFF kernel enums can be created.
};

#ifdef __cplusplus
extern "C" {
#endif

// tag::node[]
/*! \brief [Graph] This is an example ISV or OEM provided node which executes
 * in the Graph to call the ISP kernel.
 * \param [in] graph The handle to the graph in which to instantiate the node.
 * \param [in] input The input image.
 * \param [in] value The bayer pattern value
 * \param [out] output The output image.
 * \ingroup group_example_kernel
 */
vx_node vxDemosaicNode(vx_graph graph, vx_image input, vx_uint32 bayer_pattern, vx_image output);
// end::node[]

// tag::vxu[]
/*! \brief [Immediate] This is an example of an immediate mode version of the ISP node.
 * \param [in] context The overall context of the implementation.
 * \param [in] input The input image.
 * \param [in] value The bayer pattern value
 * \param [out] output The output image.
 * \ingroup group_example_kernel
 */
vx_status vxuDemosaic(vx_context context, vx_image input, vx_uint32 value, vx_image output, vx_array temp);
// end::vxu[]

#ifdef __cplusplus
}
#endif

#endif
