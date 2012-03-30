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

@synthesize startDate, startHour, startMinute, startSecond;
@synthesize timer0, vcount, vframe;

@synthesize button1, button2, button3;

@synthesize rawSeed;
@synthesize initialPIDFrame;

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  self.startDate = [NSDate date];
  self.button1 = 0;
  self.button2 = 0;
  self.button3 = 0;
}

- (NSNumber*)calcRawSeed
{
  if (!startDate || !startHour || !startMinute || !startSecond ||
      !timer0 || !vcount || !vframe)
  {
    return nil;
  }
  
  HashedSeed::Parameters  p;
  
  p.version = [gen5ConfigController version];
  p.dsType = [gen5ConfigController dsType];
  p.macAddress = [gen5ConfigController macAddress];
  p.gxStat = HashedSeed::HardResetGxStat;
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

- (IBAction)seedParameterChanged:(id)sender
{
  self.rawSeed = [self calcRawSeed];
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

- (void)setButton1:(uint32_t)newButton
{
  if (newButton != button1)
  {
    button1 = newButton;
    [self seedParameterChanged:self];
  }
}

- (void)setButton2:(uint32_t)newButton
{
  if (newButton != button2)
  {
    button2 = newButton;
    [self seedParameterChanged:self];
  }
}

- (void)setButton3:(uint32_t)newButton
{
  if (newButton != button3)
  {
    button3 = newButton;
    [self seedParameterChanged:self];
  }
}

- (void)setSeedFromResult:(id <HashedSeedResultParameters>)result
{
  HashedSeed::Parameters  p;
  
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
  self.startDate = MakeNSDate(seed.year(), seed.month(), seed.day());
  self.startHour = [NSNumber numberWithUnsignedInt: seed.hour];
  self.startMinute = [NSNumber numberWithUnsignedInt: seed.minute];
  self.startSecond = [NSNumber numberWithUnsignedInt: seed.second];
  
  self.timer0 = [NSNumber numberWithUnsignedInt: seed.timer0];
  self.vcount = [NSNumber numberWithUnsignedInt: seed.vcount];
  self.vframe = [NSNumber numberWithUnsignedInt: seed.vframe];
  
  uint32_t  button[3] = { 0, 0, 0 };
  uint32_t  i = 0;
  uint32_t  dpadPress = seed.heldButtons & Button::DPAD_MASK;
  uint32_t  buttonPress = seed.heldButtons & Button::SINGLE_BUTTON_MASK;
  
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
    if (newSeed == nil)
    {
      self.initialPIDFrame = nil;
    }
    else
    {
      HashedSeed  seed([newSeed unsignedLongLongValue]);
      self.initialPIDFrame =
        [NSNumber numberWithUnsignedInt: seed.GetSkippedPIDFrames() + 1];
    }
    
    rawSeed = newSeed;
  }
}

@end
