/*
* Copyright (c) <2024> Side Effects Software Inc.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. The name of Side Effects Software may not be used to endorse or
*    promote products derived from this software without specific prior
*    written permission.
*
* THIS SOFTWARE IS PROVIDED BY SIDE EFFECTS SOFTWARE "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
* NO EVENT SHALL SIDE EFFECTS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "CoreTypes.h"
#include "Templates/Invoke.h"

namespace Algo
{
/**
 * Heavily based on Unreal's Algo::StableRemoveIf. The difference is that the predicate is supplied
 * an index in this version, whereas in Algo::StableRemoveIf, the predicate is supplied the range
 * element.
 *
 * Moves all elements which do not match the predicate to the front of the range, while leaving all
 * other elements is a constructed but unspecified state. The elements which were not removed are
 * guaranteed to be kept in order (stable).
 *
 * @param Range The range of elements to manipulate.
 * @param Pred A callable which maps elements to truthy values, specifying elements to be removed.
 *
 * @return The index of the first element after those which were not removed.
 */
template<typename RangeType, typename Predicate>
int32 
StableRemoveIfByIndex(RangeType& Range, Predicate Pred)
{
	// We only use the index for the predicate, but not for retrieval of elements; in some range
	// types using an index is slower than an iterator.
	int32 Index = 0;

	auto* First = GetData(Range);
	auto* Last = First + GetNum(Range);

	auto* IterStart = First;

	// Skip non-removed elements at the start
	for (;;)
	{
		if (IterStart == Last)
		{
			return UE_PTRDIFF_TO_INT32(IterStart - First);
		}

		if (Invoke(Pred, Index))
		{
			break;
		}

		++IterStart;
		++Index;
	}

	auto* IterKeep = IterStart;
	++IterKeep;
	++Index;

	for (;;)
	{
		if (IterKeep == Last)
		{
			return UE_PTRDIFF_TO_INT32(IterStart - First);
		}

		if (!Invoke(Pred, Index))
		{
			*IterStart++ = MoveTemp(*IterKeep);
		}

		++IterKeep;
		++Index;
	}
}
}