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

function onFormSubmit(event) {
  var spreadSheet = SpreadsheetApp.getActiveSpreadsheet();
  var activationSheet = spreadSheet.getSheetByName('Activation');
  var accessSheet = spreadSheet.getSheetByName('Access');
  
  activationSheet.appendRow([event.namedValues.Name, event.namedValues.Surname, 'FALSE', '', '', 'FALSE', '']);
  activationSheet.appendRow([event.namedValues.Name, event.namedValues.Surname, '', 'FALSE', 'FALSE', 'FALSE', 'FALSE']);
  
  var requestDoc = docFromTemplate('Requests', 'Request template', 'Member request by %name %surname', event.namedValues);
  
  var message = {
        to: event.namedValues.address,
        replyTo: 'info@fablaquila.org',
        subject: requestDoc.getName(),
        body: 'Your precompiled member request in the attachments',
        attachments : requestDoc.getAs('application/pdf')
      };
  
  MailApp.sendEmail(message);
}

