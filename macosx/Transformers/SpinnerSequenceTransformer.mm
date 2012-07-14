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


#import "SpinnerSequenceTransformer.h"
#include "PPRNGTypes.h"

using namespace pprng;

namespace
{

static NSString* GetPositionIndicator(SpinnerPositions::Position p, BOOL first)
{
  switch (p)
  {
  case SpinnerPositions::UP:
    return first ? @"↑" : @" ↑";
  case SpinnerPositions::UP_RIGHT:
    return first ? @"↗" : @" ↗";
  case SpinnerPositions::RIGHT:
    return first ? @"→" : @" →";
  case SpinnerPositions::DOWN_RIGHT:
    return first ? @"↘" : @" ↘";
  case SpinnerPositions::DOWN:
    return first ? @"↓" : @" ↓";
  case SpinnerPositions::DOWN_LEFT:
    return first ? @"↙" : @" ↙";
  case SpinnerPositions::LEFT:
    return first ? @"←" : @" ←";
  case SpinnerPositions::UP_LEFT:
    return first ? @"↖" : @" ↖";
  default:
    return @"";
  }
}

}

@implementation SpinnerSequenceTransformer

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
  
  SpinnerPositions  spins([value unsignedLongLongValue]);
  uint32_t          numSpins = spins.NumSpins();
  
  NSString  *result = nil;
  if (numSpins > 0)
  {
    result = GetPositionIndicator(spins.GetPosition(0), true);
    
    for (uint32_t i = 1; i < numSpins; ++i)
    {
      result = [result stringByAppendingString:
                       GetPositionIndicator(spins.GetPosition(i), false)];
    }
  }
  
  return result;
}

@end
