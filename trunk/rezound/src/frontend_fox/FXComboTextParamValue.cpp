/* 
 * Copyright (C) 2002 - David W. Durham
 * 
 * This file is part of ReZound, an audio editing application.
 * 
 * ReZound is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 * 
 * ReZound is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include "FXComboTextParamValue.h"

#include <stdlib.h>

#include <algorithm> 

#include <istring>

#include <CNestedDataFile/CNestedDataFile.h>
#define DOT (CNestedDataFile::delimChar)

/*
	- This is the text entry widget used over and over by ReZound on action dialogs
	- Its purpose is to select a constant value for a parameter to an action
*/

FXDEFMAP(FXComboTextParamValue) FXComboTextParamValueMap[]=
{
	//Message_Type				ID					Message_Handler

	//FXMAPFUNC(SEL_COMMAND,			FXComboTextParamValue::ID_VALUE_SPINNER,	FXComboTextParamValue::onValueSpinnerChange),
	//FXMAPFUNC(SEL_COMMAND,			FXComboTextParamValue::ID_VALUE_TEXTBOX,	FXComboTextParamValue::onValueTextBoxChange),
};

FXIMPLEMENT(FXComboTextParamValue,FXHorizontalFrame,FXComboTextParamValueMap,ARRAYNUMBER(FXComboTextParamValueMap))

FXComboTextParamValue::FXComboTextParamValue(FXComposite *p,int opts,const char *title,const vector<string> &items,bool _isEditable) :
	FXHorizontalFrame(p,opts|FRAME_RIDGE | LAYOUT_FILL_X|LAYOUT_CENTER_Y,0,0,0,0, 6,6,2,4, 2,0),
	isEditable(_isEditable),

	titleLabel(new FXLabel(this,title,NULL,LABEL_NORMAL|LAYOUT_CENTER_Y)),
	valueComboBox(new FXComboBox(this,8,min((size_t)items.size(),(size_t)8),NULL,0, COMBOBOX_NORMAL|(!isEditable ? COMBOBOX_STATIC : 0) | FRAME_SUNKEN|FRAME_THICK | LAYOUT_CENTER_Y|LAYOUT_FILL_X))
{
	setItems(items);
}

FXComboTextParamValue::~FXComboTextParamValue()
{
}

const FXint FXComboTextParamValue::getValue()
{
	if(isEditable)
		return atoi(valueComboBox->getText().text()); // ??? I think I should just return a string.. or perhaps have a getText() method and CActionParamDialog can save both values in CActionParameters one with an altername name 
	else
		return valueComboBox->getCurrentItem();
}

void FXComboTextParamValue::setValue(const FXint value)
{
	if(isEditable)
		valueComboBox->setText(istring(value).c_str());
	else
		valueComboBox->setCurrentItem(value);
}

void FXComboTextParamValue::setItems(const vector<string> &items)
{
	FXint n=valueComboBox->getCurrentItem();

	valueComboBox->clearItems();
	for(size_t t=0;t<items.size();t++)
		valueComboBox->appendItem(items[t].c_str());

	if(n>=0)
		valueComboBox->setCurrentItem(min(n,valueComboBox->getNumItems()-1));
	else if(valueComboBox->getNumItems()>0)
		valueComboBox->setCurrentItem(0);

	valueComboBox->setNumVisible(min((size_t)8,(size_t)items.size()));
}

const string FXComboTextParamValue::getTitle() const
{
	return(titleLabel->getText().text());
}

void FXComboTextParamValue::setTipText(const FXString &text)
{
	titleLabel->setTipText(text);	
	valueComboBox->setTipText(text);
}

FXString FXComboTextParamValue::getTipText() const
{
	return(titleLabel->getTipText());	
}

void FXComboTextParamValue::readFromFile(const string &prefix,CNestedDataFile *f)
{
	const string key=prefix+DOT+getTitle()+DOT+"index";
	if(f->keyExists(key.c_str()))
	{
		const string v=f->getValue(key.c_str());
		setValue(atoi(v.c_str()));
	}
	else
		setValue(0); // ??? would use initialIndex if there were such a thing
}

void FXComboTextParamValue::writeToFile(const string &prefix,CNestedDataFile *f)
{
	const string key=prefix+DOT+getTitle()+DOT;
	f->createKey((key+"index").c_str(),istring(getValue()));
}


