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


#import "EggSeedSearcherController.h"

#import "StandardSeedInspectorController.h"
#import "SearchResultProtocols.h"

#include "EggSeedSearcher.h"
#include "Utilities.h"

using namespace pprng;

@interface EggSeedSearchResult :
  NSObject <HashedSeedResultParameters, PIDResult>
{
  DECLARE_HASHED_SEED_RESULT_PARAMETERS_VARIABLES();
  
  OptionalIVs  femaleIVs, maleIVs;
  BOOL         internationalParents, usingEverstone, usingDitto;
  
  uint32_t  startFrame, pidFrame;
  DECLARE_PID_RESULT_VARIABLES();
  BOOL      inheritsHiddenAbility;
  
  uint32_t       ivFrame;
  id             hp, atk, def, spa, spd, spe;
  Element::Type  hiddenType;
  NSNumber       *hiddenPower;
  
  FemaleParent::Type  femaleSpecies;
  NSString            *eggSpecies;
}

@property OptionalIVs  femaleIVs, maleIVs;
@property BOOL  internationalParents, usingEverstone, usingDitto;

@property uint32_t  startFrame, pidFrame;
@property BOOL      inheritsHiddenAbility;

@property uint32_t         ivFrame;
@property (copy) id        hp, atk, def, spa, spd, spe;
@property Element::Type    hiddenType;
@property (copy) NSNumber  *hiddenPower;

@property FemaleParent::Type  femaleSpecies;
@property (copy) NSString     *eggSpecies;

@end

@implementation EggSeedSearchResult

SYNTHESIZE_HASHED_SEED_RESULT_PARAMETERS_PROPERTIES();

@synthesize femaleIVs, maleIVs;
@synthesize internationalParents, usingEverstone, usingDitto;

@synthesize startFrame, pidFrame;
SYNTHESIZE_PID_RESULT_PROPERTIES();
@synthesize inheritsHiddenAbility;

@synthesize ivFrame;
@synthesize hp, atk, def, spa, spd, spe;
@synthesize hiddenType, hiddenPower;
@synthesize femaleSpecies, eggSpecies;

@end

namespace
{

static id GetEggIV(const Gen5EggFrame &frame, IVs::Type iv)
{
  if (frame.ivs.isSet(iv))
    return [NSNumber numberWithUnsignedInt: frame.ivs.iv(iv)];
  
  switch (frame.inheritance[iv])
  {
  default:
  case Gen5EggFrame::NotInherited:
    return @"??";
    
  case Gen5EggFrame::ParentX:
    return @"♀";
    
  case Gen5EggFrame::ParentY:
    return @"♂";
  }
}

struct ResultHandler
{
  ResultHandler(SearcherController *co, const EggSeedSearcher::Criteria &cr)
    : controller(co), m_criteria(cr)
  {}
  
  void operator()(const Gen5EggFrame &frame)
  {
    EggSeedSearchResult  *result = [[EggSeedSearchResult alloc] init];
    
    SetHashedSeedResultParameters(result, frame.seed);
    
    result.femaleIVs = m_criteria.femaleIVs;
    result.maleIVs = m_criteria.maleIVs;
    
    result.internationalParents =
      m_criteria.frameParameters.internationalParents;
    result.usingEverstone = m_criteria.frameParameters.usingEverstone;
    result.usingDitto = m_criteria.frameParameters.usingDitto;
    
    result.startFrame = frame.seed.GetSkippedPIDFrames() + 1;
    result.pidFrame = frame.number;
    
    SetPIDResult(result, frame.pid,
                 m_criteria.frameParameters.tid,
                 m_criteria.frameParameters.sid, frame.nature,
                 frame.pid.Gen5Ability(),
                 Gender::ANY,
                 (m_criteria.frameParameters.femaleSpecies ==
                  FemaleParent::OTHER) ?
                   Gender::ANY_RATIO : Gender::NO_RATIO);
    result.inheritsHiddenAbility = frame.inheritsHiddenAbility;
    
    result.ivFrame = frame.ivFrameNumber;
    
    result.hp = GetEggIV(frame, IVs::HP);
    result.atk = GetEggIV(frame, IVs::AT);
    result.def = GetEggIV(frame, IVs::DF);
    result.spa = GetEggIV(frame, IVs::SA);
    result.spd = GetEggIV(frame, IVs::SD);
    result.spe = GetEggIV(frame, IVs::SP);
    
    if (frame.ivs.allSet())
    {
      result.hiddenType = frame.ivs.values.HiddenType();
      result.hiddenPower =
        [NSNumber numberWithUnsignedInt: frame.ivs.values.HiddenPower()];
    }
    else
    {
      result.hiddenType = Element::NONE;
      result.hiddenPower = nil;
    }
    
    result.femaleSpecies = m_criteria.frameParameters.femaleSpecies;
    result.eggSpecies = SpeciesString(frame.species);
    
    [controller performSelectorOnMainThread: @selector(addResult:)
                withObject: result
                waitUntilDone: NO];
  }
  
  SearcherController               *controller;
  const EggSeedSearcher::Criteria  &m_criteria;
};

struct ProgressHandler
{
  ProgressHandler(SearcherController *c)
    : controller(c)
  {}
  
  bool operator()(double progressDelta)
  {
    [controller performSelectorOnMainThread: @selector(adjustProgress:)
                withObject: [NSNumber numberWithDouble: progressDelta]
                waitUntilDone: NO];
    
    return ![controller searchIsCanceled];
  }
  
  SearcherController  *controller;
};

}

@implementation EggSeedSearcherController

@synthesize fromDate, toDate;
@synthesize noButtonHeld, oneButtonHeld, twoButtonsHeld, threeButtonsHeld;

@synthesize femaleSpecies, isNidoranFemale;

@synthesize femaleHP, femaleAT, femaleDF, femaleSA, femaleSD, femaleSP;
@synthesize maleHP, maleAT, maleDF, maleSA, maleSD, maleSP;

@synthesize usingEverstone, usingDitto, internationalParents;

@synthesize startFromInitialPIDFrame;
@synthesize minPIDFrame, maxPIDFrame;
@synthesize minIVFrame, maxIVFrame;

@synthesize showShinyOnly;
@synthesize considerEggSpecies, eggSpecies;
@synthesize ability, inheritsHiddenAbility;
@synthesize gender, genderRatio;

@synthesize seedFileStatus;

- (NSString *)windowNibName
{
	return @"EggSeedSearcher";
}

- (void)loadSeedFile
{
  @synchronized (self)
  {
    EggSeedSearcher::CacheLoadResult  loadResult =
      EggSeedSearcher::LoadSeedCache();
    
    switch (loadResult)
    {
    case EggSeedSearcher::LOADED:
      seedCacheIsLoaded = YES;
      self.seedFileStatus = @"Seed cache loaded successfully. Searches for Black / White egg seeds will be approximately three times as fast.";
      break;
      
    case EggSeedSearcher::NO_CACHE_FILE:
      self.seedFileStatus = @"Failed to find seed cache file.  Please download the seed cache file and place it in the same directory as PPRNG to enable faster searching for Black / White egg seeds!";
      break;
      
    case EggSeedSearcher::BAD_CACHE_FILE:
      self.seedFileStatus = @"Seed cache file is corrupted or not of the correct format.  Please download the seed cache file again to enable faster searching for Black / White egg seeds!";
      break;
      
    case EggSeedSearcher::NOT_ENOUGH_MEMORY:
      self.seedFileStatus = @"There is not enough memory available to load the seed cache. Try closing other applications that might be using large amounts of resources.";
      break;
      
    case EggSeedSearcher::UNKNOWN_ERROR:
    default:
      self.seedFileStatus = @"Unknown error when trying to load the seed cache file!";
      break;
    }
  }
}

- (void)releaseSeedFile
{
  @synchronized (self)
  {
    if (seedCacheIsLoaded)
      EggSeedSearcher::ReleaseSeedCache();
  }
}

- (void)awakeFromNib
{
  [searcherController setGetValidatedSearchCriteriaSelector:
                      @selector(getValidatedSearchCriteria)];
  [searcherController setDoSearchWithCriteriaSelector:
                      @selector(doSearchWithCriteria:)];
  
  [[searcherController tableView] setTarget: self];
  [[searcherController tableView] setDoubleAction: @selector(inspectSeed:)];
  
  //[speciesPopUp setAutoenablesItems: NO];
  
  NSDate  *now = [NSDate date];
  self.fromDate = now;
  self.toDate = now;
  
  self.noButtonHeld = YES;
  self.oneButtonHeld = YES;
  self.twoButtonsHeld = NO;
  self.threeButtonsHeld = NO;
  
  self.femaleSpecies = FemaleParent::OTHER;
  self.isNidoranFemale = NO;
  
  self.femaleHP = nil;
  self.femaleAT = nil;
  self.femaleDF = nil;
  self.femaleSA = nil;
  self.femaleSD = nil;
  self.femaleSP = nil;
  self.maleHP = nil;
  self.maleAT = nil;
  self.maleDF = nil;
  self.maleSA = nil;
  self.maleSD = nil;
  self.maleSP = nil;
  
  self.internationalParents = NO;
  self.usingEverstone = NO;
  self.usingDitto = NO;
  
  self.startFromInitialPIDFrame = YES;
  self.minPIDFrame = 50;
  self.maxPIDFrame = 300;
  self.minIVFrame = 8;
  self.maxIVFrame = 8;
  
  self.showShinyOnly = YES;
  self.considerEggSpecies = NO;
  self.eggSpecies = EggSpecies::ANY;
  self.ability = Ability::ANY;
  self.inheritsHiddenAbility = NO;
  self.gender = Gender::ANY;
  self.genderRatio = Gender::NO_RATIO;
  
  seedCacheIsLoaded = NO;
  self.seedFileStatus = @"Loading seed cache file...";
  [self performSelectorInBackground: @selector(loadSeedFile) withObject: nil];
}

- (void)windowWillClose:(NSNotification *)notification
{
  if ([searcherController isSearching])
    [searcherController startStop: self];
  
  [self performSelectorInBackground: @selector(releaseSeedFile) withObject:nil];
}

- (void)setFemaleSpecies:(FemaleParent::Type)newFemaleSpecies
{
  if (newFemaleSpecies != femaleSpecies)
  {
    femaleSpecies = newFemaleSpecies;
    
    if (newFemaleSpecies == FemaleParent::OTHER)
    {
      self.considerEggSpecies = NO;
    }
    else
    {
      self.considerEggSpecies = YES;
      self.isNidoranFemale = (newFemaleSpecies == FemaleParent::NIDORAN_FEMALE);
    }
    
    self.gender = Gender::ANY;
    self.genderRatio = Gender::NO_RATIO;
    self.eggSpecies = EggSpecies::ANY;
  }
}

- (IBAction)toggleDropDownChoice:(id)sender
{
  HandleComboMenuItemChoice(sender);
}

- (void)inspectSeed:(id)sender
{
  NSInteger  rowNum = [sender clickedRow];
  
  if (rowNum >= 0)
  {
    EggSeedSearchResult  *row =
      [[[searcherController arrayController] arrangedObjects]
        objectAtIndex: rowNum];
    
    if (row != nil)
    {
      StandardSeedInspectorController  *inspector =
        [[StandardSeedInspectorController alloc] init];
      [inspector window];
      
      [inspector setSeedFromResult: row];
      
      inspector.selectedTabId = @"eggs";
      
      HashedSeedInspectorEggsTabController *eggsTab =
        inspector.eggsTabController;
      
      eggsTab.internationalParents = row.internationalParents;
      eggsTab.usingEverstone = row.usingEverstone;
      eggsTab.usingDitto = row.usingDitto;
      
      eggsTab.enableIVs = YES;
      eggsTab.ivFrame = row.ivFrame;
      
      if (row.pidFrame < row.startFrame)
      {
        eggsTab.startFromInitialPIDFrame = NO;
        eggsTab.minPIDFrame = 1;
      }
      eggsTab.maxPIDFrame = row.pidFrame + 20;
      
      eggsTab.femaleSpecies = row.femaleSpecies;
      
      eggsTab.enableParentIVs = YES;
      [eggsTab setFemaleIVs: row.femaleIVs];
      [eggsTab setMaleIVs: row.maleIVs];
      
      [eggsTab generateEggs: self];
      [eggsTab selectAndShowEggFrame: row.pidFrame];
      
      [inspector showWindow: self];
    }
  }
}

// dummy method for error panel
- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode
        contextInfo:(void *)contextInfo
{}

- (OptionalIVs)femaleParentIVs
{
  OptionalIVs  result;
  
  if (femaleHP)
    result.hp([femaleHP unsignedIntValue]);
  if (femaleAT)
    result.at([femaleAT unsignedIntValue]);
  if (femaleDF)
    result.df([femaleDF unsignedIntValue]);
  if (femaleSA)
    result.sa([femaleSA unsignedIntValue]);
  if (femaleSD)
    result.sd([femaleSD unsignedIntValue]);
  if (femaleSP)
    result.sp([femaleSP unsignedIntValue]);
  
  return result;
}

- (OptionalIVs)maleParentIVs
{
  OptionalIVs  result;
  
  if (maleHP)
    result.hp([maleHP unsignedIntValue]);
  if (maleAT)
    result.at([maleAT unsignedIntValue]);
  if (maleDF)
    result.df([maleDF unsignedIntValue]);
  if (maleSA)
    result.sa([maleSA unsignedIntValue]);
  if (maleSD)
    result.sd([maleSD unsignedIntValue]);
  if (maleSP)
    result.sp([maleSP unsignedIntValue]);
  
  return result;
}

- (NSValue*)getValidatedSearchCriteria
{
  using namespace boost::gregorian;
  using namespace boost::posix_time;
  
  if (!EndEditing([self window]))
    return nil;
  
  EggSeedSearcher::Criteria  criteria;
  
  criteria.seedParameters.macAddress = [gen5ConfigController macAddress];
  
  criteria.seedParameters.version = [gen5ConfigController version];
  criteria.seedParameters.dsType = [gen5ConfigController dsType];
  
  criteria.seedParameters.timer0Low = [gen5ConfigController timer0Low];
  criteria.seedParameters.timer0High = [gen5ConfigController timer0High];
  
  criteria.seedParameters.vcountLow = [gen5ConfigController vcountLow];
  criteria.seedParameters.vcountHigh = [gen5ConfigController vcountHigh];
  
  criteria.seedParameters.vframeLow = [gen5ConfigController vframeLow];
  criteria.seedParameters.vframeHigh = [gen5ConfigController vframeHigh];
  
  if (noButtonHeld)
  {
    criteria.seedParameters.heldButtons.push_back(0);  // no keys
  }
  if (oneButtonHeld)
  {
    criteria.seedParameters.heldButtons.insert
      (criteria.seedParameters.heldButtons.end(),
       Button::SingleButtons().begin(),
       Button::SingleButtons().end());
  }
  if (twoButtonsHeld)
  {
    criteria.seedParameters.heldButtons.insert
      (criteria.seedParameters.heldButtons.end(),
       Button::TwoButtonCombos().begin(),
       Button::TwoButtonCombos().end());
  }
  if (threeButtonsHeld)
  {
    criteria.seedParameters.heldButtons.insert
      (criteria.seedParameters.heldButtons.end(),
       Button::ThreeButtonCombos().begin(),
       Button::ThreeButtonCombos().end());
  }
  
  criteria.seedParameters.fromTime =
    ptime(NSDateToBoostDate(fromDate), seconds(0));
  
  criteria.seedParameters.toTime =
    ptime(NSDateToBoostDate(toDate), hours(23) + minutes(59) + seconds(59));
  
  criteria.frameParameters.usingEverstone = usingEverstone;
  criteria.frameParameters.usingDitto = usingDitto;
  criteria.frameParameters.internationalParents = internationalParents;
  criteria.frameParameters.tid = [gen5ConfigController tid];
  criteria.frameParameters.sid = [gen5ConfigController sid];
  
  criteria.ivFrame.min = minIVFrame;
  criteria.ivFrame.max = maxIVFrame;
  
  criteria.ivs.min = ivParameterController.minIVs;
  criteria.ivs.max = ivParameterController.maxIVs;
  criteria.ivs.shouldCheckMax =
    (criteria.ivs.max != IVs(31, 31, 31, 31, 31, 31));
  criteria.ivs.isRoamer = false;
  
  if (ivParameterController.considerHiddenPower)
  {
    criteria.ivs.hiddenType = ivParameterController.hiddenType;
    criteria.ivs.minHiddenPower = ivParameterController.minHiddenPower;
  }
  else
  {
    criteria.ivs.hiddenType = Element::NONE;
  }
  
  criteria.femaleIVs = [self femaleParentIVs];
  criteria.maleIVs = [self maleParentIVs];
  criteria.frameParameters.femaleSpecies = femaleSpecies;
  
  criteria.pid.natureMask = GetComboMenuBitMask(natureDropDown);
  criteria.pid.ability = ability;
  criteria.pid.gender = gender;
  criteria.pid.genderRatio = genderRatio;
  criteria.pid.startFromLowestFrame = startFromInitialPIDFrame;
  
  criteria.inheritsHiddenAbility = inheritsHiddenAbility;
  criteria.shinyOnly = showShinyOnly;
  criteria.eggSpecies = eggSpecies;
  
  criteria.pidFrame.min = minPIDFrame;
  criteria.pidFrame.max = maxPIDFrame;
  
  uint64_t  numResults = criteria.ExpectedNumberOfResults();
  
  if (numResults == 0)
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Invalid Search Parameters"];
    [alert setInformativeText:@"The Parent IVs specified cannot produce the desired Egg IVs."];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    [alert beginSheetModalForWindow:[self window] modalDelegate:self
           didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
           contextInfo:nil];
    
    return nil;
  }
  else if (numResults > 10000)
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Please Limit Search Parameters"];
    [alert setInformativeText:@"The current search parameters are expected to return more than 10,000 results. Please set more specific IVs, limit the date range, use fewer held keys, or other similar settings to reduce the number of expected results."];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    [alert beginSheetModalForWindow:[self window] modalDelegate:self
           didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
           contextInfo:nil];
    
    return nil;
  }
  else
  {
    return [NSValue valueWithPointer: new EggSeedSearcher::Criteria(criteria)];
  }
}

- (void)doSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<EggSeedSearcher::Criteria> 
    criteria(static_cast<EggSeedSearcher::Criteria*>
      ([criteriaPtr pointerValue]));
  
  EggSeedSearcher  searcher;
  
  searcher.Search(*criteria,
                  ResultHandler(searcherController, *criteria),
                  ProgressHandler(searcherController));
}


@end
