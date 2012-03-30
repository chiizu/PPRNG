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


#import <Cocoa/Cocoa.h>

#include "PPRNGTypes.h"
#include "SearchCriteria.h"
#include "HashedSeed.h"
#include "FrameGenerator.h"
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

extern boost::gregorian::date NSDateToBoostDate(NSDate *date);

extern NSDate* MakeNSDate(uint32_t year, uint32_t month, uint32_t day);

extern uint32_t MakeUInt32Date(uint32_t year, uint32_t month, uint32_t day);
extern uint32_t MakeUInt32Date(const boost::gregorian::date &d);
extern uint32_t GetUInt32DateYear(uint32_t date);
extern uint32_t GetUInt32DateMonth(uint32_t date);
extern uint32_t GetUInt32DateDay(uint32_t date);
extern NSDate* UInt32DateToNSDate(uint32_t date);
extern boost::gregorian::date UInt32DateToBoostDate(uint32_t date);

extern uint32_t MakeUInt32Time(uint32_t hour, uint32_t minute, uint32_t second);
extern uint32_t MakeUInt32Time(const boost::posix_time::time_duration &t);
extern uint32_t GetUInt32TimeHour(uint32_t time);
extern uint32_t GetUInt32TimeMinute(uint32_t time);
extern uint32_t GetUInt32TimeSecond(uint32_t time);
extern boost::posix_time::time_duration UInt32TimeToBoostTime(uint32_t time);

extern boost::posix_time::ptime
  UInt32DateAndTimeToBoostTime(uint32_t date, uint32_t time);

extern NSString* HeldItemString(pprng::HeldItem::Type t);
extern NSString* GetGen5PIDFrameDetails
  (const pprng::Gen5PIDFrame &frame,
   pprng::Gen5PIDFrameGenerator::Parameters &params);
extern NSString* GetCGearFrameTime(uint32_t ticks);
extern NSString* SpeciesString(pprng::EggSpecies::Type eggSpecies);

extern void HandleComboMenuItemChoice(NSPopUpButton *menu);
extern uint32_t GetComboMenuBitMask(NSPopUpButton *menu);

// attempt to commit any current edit
// for example, if user is typing in a text field, and then clicks a button
// this is used to commit the typed text so that it can be used in a search
extern BOOL EndEditing(NSWindow *window);

extern void SaveTableContentsToCSV(NSTableView *tableView,
                                   NSArrayController *contentArray);

extern BOOL CheckExpectedResults
  (pprng::SearchCriteria &criteria, uint64_t maxResults,
   NSString *tooManyResultsMessage, id caller, SEL alertHandler);
