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


#import "TrainerIDFormatter.h"

@implementation TrainerIDFormatter

- (NSString*)stringForObjectValue:(id)anObject
{
  if (![anObject isKindOfClass:[NSNumber class]])
  {
    return nil;
  }
  
  return [NSString stringWithFormat: @"%0.5d", [anObject unsignedIntValue]];
}

- (BOOL)getObjectValue:(id*)anObject forString:(NSString*)string
        errorDescription:(NSString**)error
{
  if ([string length] > 5)
  {
    if (error)
    {
      *error = @"Too many digits for Trainer ID";
    }
    return NO;
  }
  else if ([string length] > 0)
  {
    NSScanner  *scanner = [NSScanner scannerWithString: string];
    int        result;
    
    if ([scanner scanInt: &result] && [scanner isAtEnd] && (result >= 0))
    {
      if (result <= 65535)
      {
        if (anObject)
        {
          unsigned int  tid = result;
          
          *anObject = [NSNumber numberWithUnsignedInt: tid];
        }
        
        return YES;
      }
      else if (error)
      {
        *error = @"Trainer ID is limited to a value between 0 and 65535";
      }
    }
    else if (error)
    {
      *error = @"Not a Trainer ID";
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
  if ([partialString length] > 5)
  {
    if (error)
    {
      *error = @"Too many digits for Trainer ID";
    }
    return NO;
  }
  else if ([partialString length] > 0)
  {
    NSScanner  *scanner = [NSScanner scannerWithString: partialString];
    int        result;
    
    if ([scanner scanInt: &result] && [scanner isAtEnd] && (result >= 0))
    {
      return YES;
    }
    
    if (error)
    {
      *error = @"Not a Trainer ID";
    }
    return NO;
  }
  else
  {
    return YES;
  }
}
@end
