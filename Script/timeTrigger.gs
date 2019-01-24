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

function testTimeTrigger() {
  var testData = {
    year : 2019,
    month : 1,
    'day-of-week' : 24
  };
  onTimeTrigger(testData);
}

// called every day to check expiry date
// e-mail 1 month before expiry
// e-mail the day of expiry and deactivation of the account

function onTimeTrigger(event) {
  
  var sheet = SpreadsheetApp.getActiveSpreadsheet();
  
  var expiryDateValues = sheet.getRangeByName('expiryDate').getValues();
  var quotePaidRange = sheet.getRangeByName('quotePaid');
  var quotePaidValues = quotePaidRange.getValues();
  var mailAddressValues = sheet.getRangeByName('mailAddress').getValues();
  
  
  var dateNow = new Date(event.year, event.month - 1, event['day-of-month']);
  
  var i = 1;
  var numberToBeAdmitted = 0;
  var associateAdmitted = sheet.getRangeByName('associateAdmitted').getValues();

  while (associateAdmitted[i]) {
    if (associateAdmitted[i][0] == 'false' || associateAdmitted[i][0] == 'FALSE' || associateAdmitted[i][0] == '') {
      numberToBeAdmitted++;
    }
    i++;
  }
  if (numberToBeAdmitted > 0) {
    var users = DriveApp.getFileById(sheet.getId()).getEditors();
    var mailAddresses;
    for (var user in users) {
      mailAddresses.push(user.getEmail());
    }
    var body = 'There are ' + numberToBeAdmitted + ' pending admissions';
    MailApp.sendEmail(mailAddresses.join(), 'Pending admissions', body);
  }
  
  i = 1;
  while (quotePaidValues[i]) {
    if (quotePaidValues[i] == 'true') {
      
      var expiryDate = expiryDateValues[i][0];
      var dateOneMonthBefore = new Date(expiryDate.getFullYear(), expiryDate.getMonth() - 1, expiryDate.getDate());
      var mailAddress = mailAddressValues[i][0];

      var message = {
        to : mailAddress,
        replyTo : "info@fablaquila.org",
        subject : "Your FabLabAQ subscription"
      };
      
      if (dateNow.getTime() == dateOneMonthBefore.getTime()) {
        message.body = "Your FabLabAQ subscription expires in one month";
        MailApp.sendEmail(message);
      }
      else if (dateNow.getTime() >= expiryDate.getTime()) {
        quotePaidRange.getCell(i + 1, 1).setValue('false');
        message.body = "Your FabLabAQ subscription expired";
        MailApp.sendEmail(message);
      }
    }
    i++;
  }
}

