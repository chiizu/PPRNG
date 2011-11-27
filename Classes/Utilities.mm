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


#import "Utilities.h"

using namespace pprng;

boost::gregorian::date NSDateToBoostDate(NSDate *date)
{
  NSDateComponents *components =
    [[NSCalendar currentCalendar]
     components: (NSDayCalendarUnit | NSMonthCalendarUnit | NSYearCalendarUnit)
     fromDate: date];
  
  return boost::gregorian::date([components year], [components month],
                                [components day]);
}

NSString* NatureString(pprng::Nature::Type nature)
{
  return @"";
}

NSString* GenderString(PID pid)
{
  uint32_t  genderValue = pid.GenderValue();
  
  if (genderValue < 31)
  {
    return @"F/F/F/F";
  }
  else if (genderValue < 63)
  {
    return @"M/F/F/F";
  }
  else if (genderValue < 127)
  {
    return @"M/M/F/F";
  }
  else if (genderValue < 191)
  {
    return @"M/M/M/F";
  }
  else
  {
    return @"M/M/M/M";
  }
}

/*
NSString* GenderString(PID pid)
{
  uint32_t  genderValue = pid.GenderValue();
  
  if (genderValue < 31)
  {
    return @"♀";
  }
  else if (genderValue < 63)
  {
    return @"≥¼";
  }
  else if (genderValue < 127)
  {
    return @"≥½";
  }
  else if (genderValue < 191)
  {
    return @"≥¾";
  }
  else
  {
    return @"♂";
  }
}
*/

NSString* HeldItemString(HeldItem::Type t)
{
  switch (t)
  {
  case HeldItem::FIFTY_PERCENT_ITEM:
    return @"50%";
    break;
    
  case HeldItem::FIVE_PERCENT_ITEM:
    return @"5%";
    break;
    
  case HeldItem::ONE_PERCENT_ITEM:
    return @"1%";
    break;
    
  case HeldItem::NO_ITEM:
  default:
    return @"";
    break;
  }
}

NSString* SpeciesString(FemaleParent::Type t, uint32_t childSpecies)
{
  switch (t)
  {
  case FemaleParent::NIDORAN_FEMALE:
    return (childSpecies == 0) ? @"Nidoran ♀" : @"Nidoran ♂";
    break;
  case FemaleParent::ILLUMISE:
    return (childSpecies == 0) ? @"Volbeat" : @"Illumise";
    break;
  case FemaleParent::OTHER:
  default:
    return @"";
    break;
  }
}

void SaveTableContentsToCSV(NSTableView *tableView,
                            NSArrayController *contentArray)
{
  NSSavePanel  *sp = [NSSavePanel savePanel];
  
  [sp setRequiredFileType: @"csv"];
  
  int          runResult = [sp runModal];
  
  if (runResult != NSOKButton)
    return;
  
  NSString        *result = @"";
  
  NSArray         *columns = [tableView tableColumns];
  NSEnumerator    *columnEnumerator = [columns objectEnumerator];
  NSTableColumn   *column;
  NSMutableArray  *columnIds =
    [NSMutableArray arrayWithCapacity: [columns count]];
  NSMutableArray  *columnCells =
    [NSMutableArray arrayWithCapacity: [columns count]];
  
  /* output header row */
  column = [columnEnumerator nextObject];
  while ((column != nil) && [column isHidden])
  {
    column = [columnEnumerator nextObject];
  }
  
  if (column != nil)
  {
    [columnIds addObject: [column identifier]];
    [columnCells addObject: [column dataCell]];
    result = [result stringByAppendingString:[[column headerCell] stringValue]];
    
    while (column = [columnEnumerator nextObject])
    {
      if (![column isHidden])
      {
        [columnIds addObject: [column identifier]];
        [columnCells addObject: [column dataCell]];
        result = [result stringByAppendingFormat: @",%@",
                        [[column headerCell] stringValue]];
      }
    }
    
    result = [result stringByAppendingString: @"\n"];
  }
  
  NSArray              *rows = [contentArray arrangedObjects];
  NSEnumerator         *rowEnumerator = [rows objectEnumerator];
  NSMutableDictionary  *row;
  
  /* output data rows */
  while (row = [rowEnumerator nextObject])
  {
    columnEnumerator = [columnIds objectEnumerator];
    NSString  *columnId = [columnEnumerator nextObject];
    
    NSEnumerator  *cellEnum = [columnCells objectEnumerator];
    NSCell        *cell = [cellEnum nextObject];
    
    if (columnId != nil)
    {
      id  dataObject = [row objectForKey: columnId];
      
      [cell setObjectValue: dataObject];
      
      NSString  *data = [cell stringValue];
      
      result = [result stringByAppendingString: data];
      
      while (columnId = [columnEnumerator nextObject])
      {
        cell = [cellEnum nextObject];
        dataObject = [row objectForKey: columnId];
        
        [cell setObjectValue: dataObject];
        data = [cell stringValue];
        
        result = [result stringByAppendingFormat: @",%@", data];
      }
      
      result = [result stringByAppendingString: @"\n"];
    }
  }
  
  NSError  *error;
  [result writeToFile: [sp filename] atomically: YES
          encoding: NSUTF8StringEncoding error: &error];
}


BOOL CheckExpectedResults
  (SeedSearchCriteria &criteria, uint64_t maxResults,
   NSString *tooManyResultsMessage, id caller, SEL alertHandler)
{
  uint64_t  numResults = 0;
  NSString  *messageText = nil, *informativeText = nil;
  BOOL      failed = NO;
  
  try
  {
    numResults = criteria.ExpectedNumberOfResults();
  }
  catch (IVs::ImpossibleMinMaxIVRangeException &e)
  {
    failed = YES;
    messageText = @"Impossible IVs";
    informativeText = @"The desired IVs are not possible.  Please ensure that each minimum IV is less than or equal to each corresponding maximum IV.";
  }
  catch (IVs::ImpossibleMinHiddenPowerException &e)
  {
    failed = YES;
    messageText = @"Impossible Minimum Hidden Power";
    informativeText = @"The minimum Hidden Power specified is not possible with the desired IVs.  Please modify the desired IVs, lower the minimum Hidden Power, or disable the Hidden Power search parameters.";
  }
  catch (IVs::ImpossibleHiddenTypeException &e)
  {
    failed = YES;
    messageText = @"Impossible Hidden Power Type";
    informativeText = @"The Hidden Power type specified is not possible with the desired IVs.  Please modify the desired IVs, select a different Hidden Power type or 'Any', or disable the Hidden Power search parameters.";
  }
  catch (Exception &e)
  {
    failed = YES;
    messageText = @"Unexpected PPRNG Exception";
    informativeText = [NSString stringWithFormat: @"An unexpected PPRNG exception has occurred while verifying the search parameters: '%s'\nPlease take a screenshot of your search parameters and inform the developer.",
                       e.what()];
  }
  catch (std::exception &e)
  {
    failed = YES;
    messageText = @"Unexpected C++ Exception";
    informativeText = [NSString stringWithFormat: @"An unexpected C++ exception has occurred while verifying the search parameters: '%s'\nPlease take a screenshot of your search parameters and inform the developer.",
                       e.what()];
  }
  catch (...)
  {
    failed = YES;
    messageText = @"Unknown Exception";
    informativeText = @"An unknown exception has occurred while verifying the search parameters.  Please take a screenshot of your search parameters and inform the developer.";
  }
  
  if (!failed && (numResults > maxResults))
  {
    failed = YES;
    messageText = @"Please Limit Search Parameters";
    informativeText = tooManyResultsMessage;
  }
  
  if (failed)
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText: messageText];
    [alert setInformativeText: informativeText];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    [alert beginSheetModalForWindow: [caller window] modalDelegate: caller
           didEndSelector: alertHandler contextInfo: nil];
  }
  
  return !failed;
}
