const HEADERS = ['Event ID','Timestamp','Device ID','Source IP','Source Port','Bytes','Format','Table Number','Parsed Order','Notes','Raw Data Reference','Upload Status'];
function doPost(e) {
  try {
    const p = JSON.parse(e.postData.contents);
    const expected = PropertiesService.getScriptProperties().getProperty('CAPTURE_TOKEN');
    if (!expected || p.token !== expected) throw new Error('unauthorized');
    if (p.action === 'report') {
      const recipient = PropertiesService.getScriptProperties().getProperty('REPORT_EMAIL');
      if (!recipient || p.recipient !== recipient) throw new Error('invalid recipient');
      MailApp.sendEmail(recipient, 'Wongnai capture status', 'Firmware error count: ' + Number(p.errors || 0));
      return reply({ok:true,action:'report'});
    }
    if (!/^[a-f0-9-]{1,80}$/i.test(p.eventId)) throw new Error('invalid eventId');
    const lock = LockService.getScriptLock();
    lock.waitLock(20000);
    try {
      const sheet = SpreadsheetApp.openById(PropertiesService.getScriptProperties().getProperty('SHEET_ID')).getSheetByName('Events');
      if (!sheet) throw new Error('missing Events sheet');
      if (sheet.getLastRow() === 0) sheet.appendRow(HEADERS);
      const existing = sheet.getRange(1, 1, sheet.getLastRow(), 1).createTextFinder(p.eventId).matchEntireCell(true).findNext();
      if (existing) return reply({ok:true,eventId:p.eventId,duplicate:true});
      const raw = Utilities.base64Decode(p.rawBase64 || '');
      if (raw.length !== Number(p.bytes) || raw.length > 65536) throw new Error('invalid raw length');
      const folder = DriveApp.getFolderById(PropertiesService.getScriptProperties().getProperty('RAW_FOLDER_ID'));
      const file = folder.createFile(Utilities.newBlob(raw,'application/octet-stream',p.eventId+'.bin'));
      sheet.appendRow([p.eventId,new Date(Number(p.timestamp)*1000),p.deviceId,p.sourceIp,p.sourcePort,p.bytes,p.format,'',p.parsedOrder||'','',file.getId(),'ACK']);
      return reply({ok:true,eventId:p.eventId});
    } finally { lock.releaseLock(); }
  } catch (err) { return reply({ok:false,error:String(err)}); }
}
function reply(value) { return ContentService.createTextOutput(JSON.stringify(value)).setMimeType(ContentService.MimeType.JSON); }
function sendDailyReport() {
  const props=PropertiesService.getScriptProperties();
  const sheet=SpreadsheetApp.openById(props.getProperty('SHEET_ID')).getSheetByName('Events');
  const rows=Math.max(0,sheet.getLastRow()-1);
  MailApp.sendEmail(props.getProperty('REPORT_EMAIL'),'Wongnai capture report','Stored events: '+rows);
}
