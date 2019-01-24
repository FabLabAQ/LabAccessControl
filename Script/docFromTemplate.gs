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

// creates a new file from a template in an existing folder
// the folder should be under the same folder of the spreadsheet
// sub-folders divided by year
// returns the new file

function docFromTemplate(parentFolderName, templateFileName, nameFormatString, data) {
  
  var id = SpreadsheetApp.getActiveSpreadsheet().getId();
  var parentFolder = DriveApp.getFileById(id).getParents().next();
  var templateFolder = parentFolder.getFoldersByName(parentFolderName).next();
  
  var yearFolder;
  var yearName = new Date().getFullYear();
  var fi = templateFolder.getFoldersByName(yearName);
  
  if (fi.hasNext()) {
    yearFolder = templateFolder.getFoldersByName(yearName).next();
  }
  else {
     yearFolder = templateFolder.createFolder(yearName);
  }
  
  var docName = nameFormatString;
  for (var key in data) {
    docName.replace('%' + key, data[key]);
  }
  
  var templateFile = templateFolder.getFilesByName(templateFileName).next();
  var docId = templateFile.makeCopy(docName, yearFolder).getId();
  var newDoc = DocumentApp.openById(docId);

  for (var key in data) {
    newDoc.getBody().replaceText('%' + key, data[key]);
  }
  newDoc.saveAndClose();
  
  return newDoc;
}
