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


#import "HexadecimalFormatter.h"

@implementation HexadecimalFormatter

- (void)setFormatWidth:(NSUInteger) width
{
  format = [NSString stringWithFormat: @"%%.%dqX", width];
  [super setFormatWidth: width];
}

- (NSString*)stringForObjectValue:(id)anObject
{
  if (![anObject isKindOfClass:[NSNumber class]])
  {
    return nil;
  }
  
  return [NSString stringWithFormat: (format ? format : @"%qX"),
                                     [anObject unsignedLongLongValue]];
}

- (BOOL)getObjectValue:(id*)anObject forString:(NSString*)string
        errorDescription:(NSString**)error
{
  NSUInteger    numChars = [self formatWidth];
  
  if ((numChars > 0) && ([string length] > numChars))
  {
    if (error)
    {
      *error = @"Too many digits for format width";
    }
    return NO;
  }
  else if ([string length] > 0)
  {
    NSScanner           *scanner = [NSScanner scannerWithString: string];
    unsigned long long  result;
    
    if ([scanner scanHexLongLong: &result] && [scanner isAtEnd])
    {
      if (anObject)
      {
        *anObject = [NSNumber numberWithUnsignedLongLong: result];
      }
      
      return YES;
    }
    
    if (error)
    {
      *error = @"Not a hexadecimal number";
    }
    return NO;
  }
  else
  {
    return YES;
  }
}

- (BOOL)isPartialStringValid:(NSString *)partialString
            newEditingString:(NSString **)newString
            errorDescription:(NSString **)error
{
  NSUInteger    numChars = [self formatWidth];
  
  if ((numChars > 0) && ([partialString length] > numChars))
  {
    if (error)
    {
      *error = @"Too many digits for format width";
    }
    return NO;
  }
  else if ([partialString length] > 0)
  {
    NSScanner           *scanner = [NSScanner scannerWithString: partialString];
    unsigned long long  result;
    
    if ([scanner scanHexLongLong: &result] && [scanner isAtEnd])
    {
      return YES;
    }
    
    if (error)
    {
      *error = @"Not a hexadecimal number";
    }
    return NO;
  }
  else
  {
    return YES;
  }
}
@end
