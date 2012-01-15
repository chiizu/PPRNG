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


#import <Cocoa/Cocoa.h>

#include "PPRNGTypes.h"
#include "SearchCriteria.h"
#include <boost/date_time/gregorian/gregorian_types.hpp>

extern boost::gregorian::date NSDateToBoostDate(NSDate *date);

extern NSString* NatureString(pprng::Nature::Type nature);
extern NSString* GenderString(pprng::PID pid);
extern NSString* HeldItemString(pprng::HeldItem::Type t);
extern NSString* SpeciesString(pprng::FemaleParent::Type t,
                               uint32_t childSpecies);

extern void SaveTableContentsToCSV(NSTableView *tableView,
                                   NSArrayController *contentArray);

extern BOOL CheckExpectedResults
  (pprng::SearchCriteria &criteria, uint64_t maxResults,
   NSString *tooManyResultsMessage, id caller, SEL alertHandler);
