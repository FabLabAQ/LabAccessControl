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

function handleActivation(sheet, event) {

  var quotePaidRange = sheet.getRangeByName('quotePaid'); 
  // check if the sheet being edited is the one we are interested in
  if (event.range.getSheet().getSheetName() == quotePaidRange.getSheet().getSheetName()
    // check if the cell being edited belongs to the column
    && event.range.getColumn() == quotePaidRange.getColumn()
    // finally check if the cell changed from false to true
    && (event.value == 'true' || event.value == 'TRUE')
    ) {
      var locale = sheet.getSpreadsheetLocale();
      var rowIndex = event.range.getRow() - 1;
      var dateNow = new Date();
      var expiryDateRange = sheet.getRangeByName('expiryDate');
      var expiryDateValues = expiryDateRange.getValues();
      var expiryDate = new Date(expiryDateValues[rowIndex][0]);
      var newExpiryDate;
      
      if (expiryDate.getTime() > dateNow.getTime()) {
        newExpiryDate = new Date(dateNow.getFullYear() + 1, expiryDate.getMonth(), expiryDate.getDate());
      }
      else {
        newExpiryDate = new Date(dateNow.getFullYear() + 1, dateNow.getMonth(), dateNow.getDate());
      }
      
      expiryDateRange.getCell(rowIndex + 1, 1).setValue(newExpiryDate.toLocaleDateString(locale));
      
      var quoteReceiptSheet = sheet.getSheetByName('Active associates');
      var receiptNumberRange = sheet.getRangeByName('receiptNumber');
      
      var lastRow = receiptNumberRange.getLastRow() - 1;
      var lastReceiptNumber = new Number(receiptNumberRange.getValues()[lastRow][0]);
      var lastReceiptDate = new Date(sheet.getRangeByName('receiptDate').getValues()[lastRow][0]);
      
      var recipientName = sheet.getRangeByName('associateName').getValues()[rowIndex][0];
      var recipientSurname = sheet.getRangeByName('associateSurname').getValues()[rowIndex][0];
      var mailAddress = sheet.getRangeByName('mailAddress').getValues()[rowIndex][0];
      
      
      
      var receiptNumber;
      var yearNow = dateNow.getFullYear();
      var yearLast = lastReceiptDate.getFullYear();
      if (lastReceiptNumber == 0 || isNaN(lastReceiptNumber) || yearNow > yearLast) {
        quoteReceiptSheet.appendRow([yearNow]);
        quoteReceiptSheet.appendRow(['Name', 'Surname', 'Birth place', 'Birth date', 'Identification code', 'Receipt date', 'Receipt number', 'Receipt link']);
        receiptNumber = 1;
      }
      else {
        receiptNumber = lastReceiptNumber + 1;
      }
      
      var data = {
        name : recipientName,
        surname : recipientSurname,
        address : mailAddress,
        date : dateNow.toLocaleDateString(locale),
        amount : '40,00',
        description : 'Subscription quote payment',
        number : receiptNumber.toString()
      };
      
      var receipt = docFromTemplate('Receipts', 'Receipt template', 'Quote receipt N.%number/Q of %date for associate %name %surname', data);
      
      
      var message = {
        to: data.address,
        replyTo: 'info@fablaquila.org',
        subject: docName,
        body: docName + ' in the attachments',
        attachments : receipt.getAs('application/pdf')
      };
  
      MailApp.sendEmail(message);
  
      quoteReceiptSheet.appendRow([data.name, data.surname, data.date, data.number, receipt.getUrl()]);
      SpreadsheetApp.flush();
  }
}

