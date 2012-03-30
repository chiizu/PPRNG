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


#import "ProfElmResponsesTransformer.h"
#include "PPRNGTypes.h"

using namespace pprng;

@implementation ProfElmResponsesTransformer

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
  
  if (![value respondsToSelector: @selector(unsignedLongLongValue)])
  {
    [NSException
     raise: NSInternalInconsistencyException
     format: @"Value (%@) does not respond to -unsignedLongLongValue.",
             [value class]];
  }
  
  ProfElmResponses  responses([value unsignedLongLongValue]);
  uint32_t          numResponses = responses.NumResponses();
  
  NSString  *result = nil;
  if (numResponses > 0)
  {
    switch (responses.GetResponse(0))
    {
      case ProfElmResponses::EVOLUTION: result = @"E"; break;
      case ProfElmResponses::KANTO: result = @"K"; break;
      default: result = @"P"; break;
    }
    
    for (uint32_t i = 1; i < numResponses; ++i)
    {
      NSString  *r;
      switch (responses.GetResponse(i))
      {
        case ProfElmResponses::EVOLUTION: r = @", E"; break;
        case ProfElmResponses::KANTO: r = @", K"; break;
        default: r = @", P"; break;
      }
      result = [result stringByAppendingString: r];
    }
  }
  
  return result;
}

@end
