/*
  Copyright (C) 2011-2012 chiizu
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
    [[[NSCalendar alloc] initWithCalendarIdentifier: NSGregorianCalendar]
     components: (NSDayCalendarUnit | NSMonthCalendarUnit | NSYearCalendarUnit)
     fromDate: date];
  
  return boost::gregorian::date([components year], [components month],
                                [components day]);
}

NSDate* MakeNSDate(uint32_t year, uint32_t month, uint32_t day)
{
  NSDateComponents  *components = [[NSDateComponents alloc] init];
  [components setYear: year];
  [components setMonth: month];
  [components setDay: day];
  
  return [[[NSCalendar alloc] initWithCalendarIdentifier: NSGregorianCalendar]
          dateFromComponents: components];
}

uint32_t MakeUInt32Date(uint32_t year, uint32_t month, uint32_t day)
{
  return (year * 10000) + (month * 100) + day;
}

uint32_t MakeUInt32Date(const boost::gregorian::date &d)
{
  return MakeUInt32Date(d.year(), d.month(), d.day());
}

uint32_t GetUInt32DateYear(uint32_t date)
{
  return date / 10000;
}

uint32_t GetUInt32DateMonth(uint32_t date)
{
  return (date % 10000) / 100;
}

uint32_t GetUInt32DateDay(uint32_t date)
{
  return date % 100;
}

NSDate* UInt32DateToNSDate(uint32_t date)
{
  return MakeNSDate(GetUInt32DateYear(date), GetUInt32DateMonth(date),
                    GetUInt32DateDay(date));
}

boost::gregorian::date UInt32DateToBoostDate(uint32_t date)
{
  return boost::gregorian::date(GetUInt32DateYear(date),
                                GetUInt32DateMonth(date),
                                GetUInt32DateDay(date));
}

uint32_t MakeUInt32Time(uint32_t hour, uint32_t minute, uint32_t second)
{
  return (hour * 10000) + (minute * 100) + second;
}

uint32_t MakeUInt32Time(const boost::posix_time::time_duration &t)
{
  return MakeUInt32Time(t.hours(), t.minutes(), t.seconds());
}

uint32_t GetUInt32TimeHour(uint32_t time)
{
  return time / 10000;
}

uint32_t GetUInt32TimeMinute(uint32_t time)
{
  return (time % 10000) / 100;
}

uint32_t GetUInt32TimeSecond(uint32_t time)
{
  return time % 100;
}

boost::posix_time::time_duration UInt32TimeToBoostTime(uint32_t time)
{
  return boost::posix_time::time_duration
    (boost::posix_time::hours(GetUInt32TimeHour(time)) +
     boost::posix_time::minutes(GetUInt32TimeMinute(time)) +
     boost::posix_time::seconds(GetUInt32TimeSecond(time)));
}

boost::posix_time::ptime
  UInt32DateAndTimeToBoostTime(uint32_t date, uint32_t time)
{
  return boost::posix_time::ptime(UInt32DateToBoostDate(date),
                                  UInt32TimeToBoostTime(time));
}

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

NSString* GetCGearFrameTime(uint32_t ticks)
{
  if (ticks == 0)
  {
    return @"Skipped";
  }
  else
  {
    uint32_t  minutes = ticks / 3600;
    ticks -= minutes * 3600;
    uint32_t  seconds = ticks / 60;
    ticks -= seconds * 60;
    
    return [NSString stringWithFormat: @"%.2d:%.2d.%.3d",
            minutes, seconds, (ticks * 1000) / 60];
  }
}

NSString* GetGen5PIDFrameDetails(const Gen5PIDFrame &frame,
                                 Gen5PIDFrameGenerator::Parameters &params,
                                 uint32_t cgearTicks)
{
  switch (params.frameType)
  {
  case Gen5PIDFrameGenerator::FishingFrame:
    return frame.isEncounter ? @"Fish" : @"";
    
  case Gen5PIDFrameGenerator::SwirlingDustFrame:
  case Gen5PIDFrameGenerator::BridgeShadowFrame:
    return frame.isEncounter ? @"" :
      [NSString stringWithFormat: @"%s",
       EncounterItem::ToString(frame.encounterItem).c_str()];
    
    
  case Gen5PIDFrameGenerator::EntraLinkFrame:
    return GetCGearFrameTime(cgearTicks);
    
  default:
    return @"";
  }
}

NSString* SpeciesString(EggSpecies::Type eggSpecies)
{
  switch (eggSpecies)
  {
  case EggSpecies::OTHER:
  default:
    return @"";
    
  case EggSpecies::NIDORAN_F:
    return @"Nidoran ♀";
    
  case EggSpecies::NIDORAN_M:
    return @"Nidoran ♂";
  case EggSpecies::VOLBEAT:
    return @"Volbeat";
  case EggSpecies::ILLUMISE:
    return @"Illumise";
  }
}


void HandleComboMenuItemChoice(NSPopUpButton *menu)
{
  NSMenuItem  *selectedItem = [menu selectedItem];
  
  if ([selectedItem tag] >= 0)
  {
    [selectedItem setState: ![selectedItem state]];
  }
  else if ([selectedItem tag] != -5)
  {
    NSInteger  action = [selectedItem tag];
    NSInteger  numItems = [menu numberOfItems];
    NSInteger  i;
    
    for (i = 0; i < numItems; ++i)
    {
      NSMenuItem  *item = [menu itemAtIndex: i];
      NSInteger   tag = [item tag];
      
      if (tag >= 0)
      {
        switch (action)
        {
        case -1:
          [item setState: NSOnState];
          break;
        case -2:
          [item setState: NSOffState];
          break;
        case -3:
          if (tag & 0x1)
            [item setState: NSOnState];
          else
            [item setState: NSOffState];
          break;
        case -4:
          if (tag & 0x1)
            [item setState: NSOffState];
          else
            [item setState: NSOnState];
          break;
        default:
          break;
        }
      }
    }
  }
}


uint32_t GetComboMenuBitMask(NSPopUpButton *menu)
{
  uint32_t   mask = 0;
  NSInteger  numItems = [menu numberOfItems];
  NSInteger  i;
  
  for (i = 0; i < numItems; ++i)
  {
    NSMenuItem  *item = [menu itemAtIndex: i];
    NSInteger   tag = [item tag];
    
    if ((tag >= 0) && ([item state] == NSOnState))
    {
      mask |= 0x1 << tag;
    }
  }
  
  return mask;
}


// adapted from
// http://blog.mbcharbonneau.com/2006/12/17/end-editing-in-an-nstextfield/
BOOL EndEditing(NSWindow *window)
{
  id  responder = [window firstResponder];

  // If we're dealing with the field editor, the real first responder is
  // its delegate.
  if ((responder != nil) &&
      [responder isKindOfClass:[NSTextView class]] &&
      [(NSTextView*)responder isFieldEditor])
    responder = ([[responder delegate] isKindOfClass:[NSResponder class]]) ?
      [responder delegate] :
      nil;

  BOOL  success = [window makeFirstResponder:nil];

  // Return first responder status.
  if (success && responder != nil)
    [window makeFirstResponder: responder];

  return success;
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
  (SearchCriteria &criteria, uint64_t maxResults,
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
