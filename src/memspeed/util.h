/* $Id: util.h,v 1.1 2002/08/07 16:14:09 phaller Exp $ */

#ifndef _PROFASM_H_
#define _PROFASM_H_

#ifdef __cplusplus
extern "C" {
#endif



// --------------------------------------------------------------------------
// Assembler callout prototypes
// --------------------------------------------------------------------------

void _System ProfileGetTimestamp(unsigned long* hi, unsigned long* lo);

  
#ifdef __cplusplus
}
#endif


#endif /* _PROFASM_H */
