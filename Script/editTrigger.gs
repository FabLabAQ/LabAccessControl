/*
 * Lab Access System
 * (c) 2019 Luca Anastasio
 * anastasio.lu@gmail.com
 * www.fablaquila.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

// called when user edits the sheet
// DON'T CHENGE THE FUNCTION'S NAME!!!
// otherwise it will be called twice!

function onEditTrigger(event) {
  var ui = SpreadsheetApp.getUi();
  try {
  
  var result = ui.alert('Confirm changes?', ui.ButtonSet.YES_NO);
  
  if (result == ui.Button.YES) {
    
    var sheet = SpreadsheetApp.getActiveSpreadsheet();
    var locale = sheet.getSpreadsheetLocale();
    var dateNow = new Date();
    event.range.setNote('Modified by ' + event.user + ' on ' + dateNow.toLocaleString(locale));

    handleActivation(sheet, event);
    handleAdmission(sheet, event);
    
  }
  else {
    event.range.setValue(event.oldValue);
  }
  }
  catch (error) {
    ui.alert('An error occourred', error.toString(), ui.ButtonSet.OK);
  }
}

