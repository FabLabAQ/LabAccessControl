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

function handleAdmission(sheet, event) {
  
  var admissionRange = sheet.getRangeByName('associateAdmitted');
  // check if the sheet being edited is the one we are interested in
  if (event.range.getSheet().getSheetName() == admissionRange.getSheet().getSheetName()
    // check if the cell being edited belongs to the column
    && event.range.getColumn() == admissionRange.getColumn()
    // finally check if the cell changed from false to true
    && (event.value == 'true' || event.value == 'TRUE')
    ) {
      var locale = sheet.getSpreadsheetLocale();
      var rowIndex = event.range.getRow() - 1;
      var dateSubmitted = new Date(sheet.getRangeByName('submittedDate').getValues()[rowIndex][0]);
      dateSubmitted = new Date(dateSubmitted.getFullYear(), dateSubmitted.getMonth(), dateSubmitted.getDate());
      var dateNow = new Date();
      dateNow = new Date(dateNow.getFullYear(), dateNow.getMonth(), dateNow.getDate());
      
      // at least one day passed
      if (dateNow.getTime() > dateSubmitted.getTime()) {
        
        sheet.getRangeByName('admissionDate').getCell(rowIndex + 1, 1).setValue(dateNow.toLocaleDateString(locale));
        
        var data = {
          name : sheet.getRangeByName('associateName').getValues()[rowIndex][0],
          surname : sheet.getRangeByName('associateSurname').getValues()[rowIndex][0],
          birthPlace : sheet.getRangeByName('birthPlace').getValues()[rowIndex][0],
          birthDate : sheet.getRangeByName('birthDate').getValues()[rowIndex][0].toLocaleDateString(locale),
          identificationCode : sheet.getRangeByName('identificationCode').getValues()[rowIndex][0],
          admissionDate : dateNow.toLocaleDateString(locale)
        }
        
        var admissionDoc = docFromTemplate('Admissions', 'Admission template', 'Admission of %name %surname', data);
  
        var message = {
          to: sheet.getRangeByName('mailAddress').getValues()[rowIndex][0],
          replyTo: 'info@fablaquila.org',
          subject: docName,
          body: 'You have been admitted, you may now pay the subscription quote.',
        };
  
        MailApp.sendEmail(message);
        
        var url = admission.getUrl();
        sheet.getRangeByName('admissionLink').getCell(rowIndex + 1, 1).setValue(url);
        
        var js = "<script> window.open('" + url + "', '_blank'); google.script.host.close(); </script>";
        var html = HtmlService.createHtmlOutput(js).setHeight(10).setWidth(100);
        SpreadsheetApp.getUi().showModalDialog(html, 'Now loading.');
        
      }
      else {
        event.range.setValue(event.oldValue);
      }
    }
}

