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



#import "HashedSeedInspectorController.h"

#include "HashedSeed.h"
#include "FrameGenerator.h"
#include "Utilities.h"

using namespace pprng;

@implementation HashedSeedInspectorController

@synthesize configurationNames;

@synthesize dsType;
@synthesize macAddress0, macAddress1, macAddress2;
@synthesize macAddress3, macAddress4, macAddress5;

@synthesize version;
@synthesize tid, sid;
@synthesize memoryLinkUsed, hasShinyCharm;

@synthesize startDate, startHour, startMinute, startSecond;
@synthesize timer0, vcount, vframe;

@synthesize button1, button2, button3;

@synthesize rawSeed;
@synthesize initialPIDFrame;

- (void)setConfig:(NSInteger)configIndex
{
  NSArray  *configurations = [[NSUserDefaults standardUserDefaults]
                              arrayForKey: @"gen5Configurations"];
  NSDictionary  *config = [configurations objectAtIndex: configIndex];
  
  self.dsType = DS::Type([[config objectForKey: @"dsType"] intValue]);
  self.macAddress0 = [config objectForKey: @"macAddress0"];
  self.macAddress1 = [config objectForKey: @"macAddress1"];
  self.macAddress2 = [config objectForKey: @"macAddress2"];
  self.macAddress3 = [config objectForKey: @"macAddress3"];
  self.macAddress4 = [config objectForKey: @"macAddress4"];
  self.macAddress5 = [config objectForKey: @"macAddress5"];
  self.version = Game::Version([[config objectForKey: @"version"] intValue]);
  
  self.tid = [config objectForKey: @"tid"];
  self.sid = [config objectForKey: @"sid"];
  
  self.memoryLinkUsed = [[config objectForKey: @"memoryLinkUsed"] boolValue];
  self.hasShinyCharm = [[config objectForKey: @"hasShinyCharm"] boolValue];
  
  self.timer0 = [config objectForKey: @"timer0Low"];
  self.vcount = [config objectForKey: @"vcountLow"];
  self.vframe = [config objectForKey: @"vframeLow"];
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  NSArray  *configurations = [[NSUserDefaults standardUserDefaults]
                              arrayForKey: @"gen5Configurations"];
  
  NSMutableArray *names =
    [NSMutableArray arrayWithCapacity: [configurations count] + 1];
  
  [names addObject: @"Use Configuration..."];
  [names addObjectsFromArray:
         [configurations valueForKeyPath: @"@unionOfObjects.name"]];
  
  self.configurationNames = names;
  
  NSInteger  currentConfigIndex =
    [[NSUserDefaults standardUserDefaults] integerForKey: @"gen5ConfigIndex"];
  
  [self setConfig: currentConfigIndex];
  
  self.startDate = [NSDate date];
  self.button1 = 0;
  self.button2 = 0;
  self.button3 = 0;
}

- (uint64_t)macAddress
{
  uint32_t  macAddressLow = [macAddress0 unsignedIntValue] |
                            ([macAddress1 unsignedIntValue] << 8) |
                            ([macAddress2 unsignedIntValue] << 16);
  uint32_t  macAddressHigh = [macAddress3 unsignedIntValue] |
                             ([macAddress4 unsignedIntValue] << 8) |
                             ([macAddress5 unsignedIntValue] << 16);
  
  return (uint64_t(macAddressHigh) << 24) | macAddressLow;
}

- (void)setMACAddress:(uint64_t)macAddress
{
  self.macAddress0 = [NSNumber numberWithUnsignedInt:macAddress & 0xff];
  self.macAddress1 =
    [NSNumber numberWithUnsignedInt:(macAddress >> 8) & 0xff];
  self.macAddress2 =
    [NSNumber numberWithUnsignedInt:(macAddress >> 16) & 0xff];
  
  self.macAddress3 =
    [NSNumber numberWithUnsignedInt:(macAddress >> 24) & 0xff];
  self.macAddress4 =
    [NSNumber numberWithUnsignedInt:(macAddress >> 32) & 0xff];
  self.macAddress5 =
    [NSNumber numberWithUnsignedInt:(macAddress >> 40) & 0xff];
}

- (NSNumber*)calcRawSeed
{
  if (!startDate || !startHour || !startMinute || !startSecond ||
      !timer0 || !vcount || !vframe)
  {
    return nil;
  }
  
  HashedSeed::Parameters  p;
  
  p.version = version;
  p.dsType = dsType;
  p.macAddress = [self macAddress];
  p.vcount = [vcount unsignedIntValue];
  p.vframe = [vframe unsignedIntValue];
  p.timer0 = [timer0 unsignedIntValue];
  p.date = NSDateToBoostDate(startDate);
  p.hour = [startHour unsignedIntValue];
  p.minute = [startMinute unsignedIntValue];
  p.second = [startSecond unsignedIntValue];
  p.heldButtons = button1 | button2 | button3;
  
  HashedSeed  s(p);
  
  return [NSNumber numberWithUnsignedLongLong: s.rawSeed];
}

- (void)calcInitialPIDFrame
{
  if (rawSeed == nil)
  {
    self.initialPIDFrame = nil;
  }
  else
  {
    HashedSeed  seed(version, [rawSeed unsignedLongLongValue]);
    self.initialPIDFrame =
      [NSNumber numberWithUnsignedInt:
        seed.GetSkippedPIDFrames(memoryLinkUsed) + 1];
  }
}

- (IBAction) configChanged:(id)sender
{
  NSInteger  selectedIdx = [sender indexOfSelectedItem];
  
  [self setConfig: selectedIdx - 1];
}

- (IBAction)seedParameterChanged:(id)sender
{
  self.rawSeed = [self calcRawSeed];
}

- (IBAction) memoryLinkUsedChanged:(id)sender
{
  [self calcInitialPIDFrame];
}

- (IBAction) seedValueChanged:(id)sender
{
  NSNumber  *paramSeed = [self calcRawSeed];
  
  if ((paramSeed == nil) || ![paramSeed isEqualToNumber: rawSeed])
  {
    self.startDate = [NSDate date];
    self.startHour = nil;
    self.startMinute = nil;
    self.startSecond = nil;
    self.timer0 = nil;
    self.vcount = nil;
    self.vframe = nil;
    self.button1 = 0;
    self.button2 = 0;
    self.button3 = 0;
  }
}

- (void)setSeedFromResult:(id <HashedSeedResultParameters>)result
{
  HashedSeed::Parameters  p;
  
  p.dsType = result.dsType;
  p.macAddress = result.macAddress;
  p.version = result.version;
  p.timer0 = result.timer0;
  p.vcount = result.vcount;
  p.vframe = result.vframe;
  p.date = UInt32DateToBoostDate(result.date);
  p.hour = GetUInt32TimeHour(result.time);
  p.minute = GetUInt32TimeMinute(result.time);
  p.second = GetUInt32TimeSecond(result.time);
  p.heldButtons = result.heldButtons;
  
  [self setSeed: HashedSeed(p, result.rawSeed)];
}

- (void)setSeed:(const pprng::HashedSeed&)seed
{
  self.dsType = seed.parameters.dsType;
  [self setMACAddress:seed.parameters.macAddress];
  
  self.version = seed.parameters.version;
  
  self.startDate = MakeNSDate(seed.year(), seed.month(), seed.day());
  self.startHour = [NSNumber numberWithUnsignedInt:seed.parameters.hour];
  self.startMinute = [NSNumber numberWithUnsignedInt:seed.parameters.minute];
  self.startSecond = [NSNumber numberWithUnsignedInt:seed.parameters.second];
  
  self.timer0 = [NSNumber numberWithUnsignedInt:seed.parameters.timer0];
  self.vcount = [NSNumber numberWithUnsignedInt:seed.parameters.vcount];
  self.vframe = [NSNumber numberWithUnsignedInt:seed.parameters.vframe];
  
  uint32_t  button[3] = { 0, 0, 0 };
  uint32_t  i = 0;
  uint32_t  dpadPress = seed.parameters.heldButtons & Button::DPAD_MASK;
  uint32_t  buttonPress =
    seed.parameters.heldButtons & Button::SINGLE_BUTTON_MASK;
  
  if (dpadPress != 0)
  {
    button[0] = dpadPress;
    ++i;
  }
  
  uint32_t  j = 1;
  while ((buttonPress != 0) && (i < 3))
  {
    if (buttonPress & 0x1)
    {
      button[i++] = j;
    }
    
    buttonPress >>= 1;
    j <<= 1;
  }
  
  self.button1 = button[0];
  self.button2 = button[1];
  self.button3 = button[2];
  
  [self seedParameterChanged:self];
}

- (void)setRawSeed:(NSNumber*)newSeed
{
  if (newSeed != rawSeed)
  {
    rawSeed = newSeed;
    [self calcInitialPIDFrame];
  }
}

@end
