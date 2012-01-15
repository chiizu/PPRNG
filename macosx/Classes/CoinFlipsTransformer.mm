/*
  Copyright (C) 2011 chiizu
  chiizu.pprng@gmail.com
  
  This file is part of PPRNG.
  
  PPRNG is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  PPRNG is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with PPRNG.  If not, see <http://www.gnu.org/licenses/>.
*/


#import "CoinFlipsTransformer.h"
#include "PPRNGTypes.h"

using namespace pprng;

@implementation CoinFlipsTransformer

+ (Class)transformedValueClass
{
  return [NSString class];
}

+ (BOOL)allowsReverseTransformation
{
  return NO;
}

- (id)transformedValue:(id)value
{
  if (value == nil)
    return nil;
  
  if (![value respondsToSelector: @selector(unsignedLongValue)])
  {
    [NSException raise: NSInternalInconsistencyException
                 format: @"Value (%@) does not respond to -unsignedLongValue.",
                         [value class]];
  }
  
  CoinFlips  flips([value unsignedLongValue]);
  uint32_t   numFlips = flips.NumFlips();
  
  NSString  *result = nil;
  if (numFlips > 0)
  {
    result = (flips.FlipResult(0) == CoinFlips::HEADS) ? @"H" : @"T";
    for (uint32_t i = 1; i < numFlips; ++i)
    {
      result = [result stringByAppendingString:
                       (flips.FlipResult(i) == CoinFlips::HEADS) ?
                       @", H" : @", T"];
    }
  }
  
  return result;
}

@end
