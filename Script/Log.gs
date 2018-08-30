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

// NOTE: Requires ArrayLib

// get log sheet handle
var logSheet = SpreadsheetApp.openById('--- Your log sheet ID here ---');
var accessSheet = SpreadsheetApp.openById('--- Your access sheet ID here ---');

function doPost(e) {
  
  Logger.log('e: %s', e);
  if (e.queryString != 'log') {
    return ContentService.createTextOutput("Not a log query");
  }
  var jsonData;
  try {
    // try to get json from post data
    jsonData = JSON.parse(e.postData.contents);
    Logger.log('jsonData: %s', jsonData);
  } 
  catch(err){
    return ContentService.createTextOutput("Error in parsing request body: " + err.message);
  }
   
  if (jsonData !== undefined){
    
    var index = ArrayLib.indexOf(accessSheet.getRangeByName('card_uids').getValues(), -1, jsonData.uid);
    
    var dataToAppend = [ jsonData.board_id,
                         Utilities.formatDate(new Date(jsonData.time*1000), "GMT", "dd/MM/yyyy HH:mm:ss"),
                         jsonData.msg,
                         (jsonData.uid != undefined) ? jsonData.uid : '',
                         (jsonData.uid != undefined) ? accessSheet.getRangeByName('user_name').getValues()[index]+" "+accessSheet.getRangeByName('user_surname').getValues()[index] : ''
    ];
    
    Logger.log('dataToAppend: %s', dataToAppend);
    
    logSheet.getSheetByName(jsonData.board_id).appendRow(dataToAppend);
    SpreadsheetApp.flush(); 
    return ContentService.createTextOutput(jsonData);
  } // endif (jsonData !== undefined)
  else{
    return ContentService.createTextOutput("Error! Request body empty or in incorrect format.");
  }
}
