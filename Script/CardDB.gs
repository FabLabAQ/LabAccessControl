/*
 * Lab Access System
 * (c) 2018 Luca Anastasio
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

var AccessSheet = SpreadsheetApp.openById('--- Your access sheet ID here ---');
                    
function doGet(e){
  
  
  var BoardName;
  
  try {
    BoardName = e.parameter.boardName;
    //BoardName = "MainDoor";
  }
  catch(err) {
    console.log("error in GET request: " + err.message);
    return;
  }
  
  if (BoardName != undefined) {
    var UIDlist = 'UIDSTART';
    var UIDcolumn = AccessSheet.getRangeByName('card_uids').getValues();
    Logger.log('UIDcolumn: %s', UIDcolumn);
    var paidColumn = AccessSheet.getRangeByName('quote_paid').getValues();
    Logger.log('paidColumn: %s', paidColumn);
    var columnToCheck = AccessSheet.getRangeByName('auth_'+BoardName).getValues();
    Logger.log('columnToCheck: %s', columnToCheck);
    
    var i = 0;
    while (columnToCheck[i]) {
      Logger.log('columnToCheck[i]: %s', columnToCheck[i]);
      if (columnToCheck[i] == 'true' && paidColumn[i] == 'true') {
        UIDlist += UIDcolumn[i];
      }
      i++;
    }
    
    Logger.log('UIDlist returned: %s', UIDlist + 'UID_STOP');
    return ContentService.createTextOutput(UIDlist + 'UID_STOP');
  }
}