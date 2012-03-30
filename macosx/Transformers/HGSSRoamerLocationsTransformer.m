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


#import "HGSSRoamerLocationsTransformer.h"

@implementation HGSSRoamerLocationsTransformer

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
  
  unsigned int  locations = [value unsignedLongValue];
  NSString  *result = @"";
  
  unsigned int  location = locations & 0x3F;
  if (location > 0)
    result = [result stringByAppendingFormat: @"R: %d  ", location];
  
  location = (locations >> 6) & 0x3F;
  if (location > 0)
    result = [result stringByAppendingFormat: @"E: %d  ", location];
  
  location = (locations >> 12) & 0x3F;
  if (location > 0)
    result = [result stringByAppendingFormat: @"L: %d", location];
  
  return result;
}

@end
