/*
  Copyright (C) 2012 chiizu
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


#import "UInt32TimeTransformer.h"

#include "Utilities.h"

using namespace pprng;

@implementation UInt32TimeTransformer

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
  
  if (![value respondsToSelector: @selector(unsignedIntValue)])
  {
    [NSException raise: NSInternalInconsistencyException
                 format: @"Value (%@) does not respond to -intValue.",
                         [value class]];
  }
  
  uint32_t  time = [value unsignedIntValue];
  
  return [NSString stringWithFormat: @"%.2d:%.2d:%.2d",
          GetUInt32TimeHour(time), GetUInt32TimeMinute(time),
          GetUInt32TimeSecond(time)];
}

@end
