/*
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

package org.fablaquila.cardmaker;

import org.fablaquila.cardmaker.MifareUltralightAuth.AuthRetCode;
import org.json.JSONObject;


import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Looper;
import android.security.keystore.KeyProperties;
import android.support.v4.app.Fragment;
import android.os.Bundle;
import android.text.InputType;
import android.util.Base64;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;
import android.support.design.widget.Snackbar;
import android.app.Activity;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.nfc.NfcAdapter;
import android.nfc.Tag;
import android.nfc.tech.MifareUltralight;
import android.content.Context;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.Console;
import java.io.DataOutputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.lang.Runnable;
import java.io.IOException;
import java.net.URL;
import java.security.KeyStore;
import java.security.MessageDigest;

import javax.crypto.Mac;
import javax.crypto.SecretKey;
import javax.net.ssl.HttpsURLConnection;

public class MainTab extends Fragment {

    private final MifareUltralightKey DefaultKey = new MifareUltralightKey(new byte[]{'B', 'R', 'E', 'A', 'K', 'M', 'E', 'I', 'F', 'Y', 'O', 'U', 'C', 'A', 'N', '!'});
    private TextView textOutput;
    private Button CopyToClipboardBtn;
    private View view;
    private ClipboardManager clipboard;
    private Context context;
    private NfcAdapter nfc;
    private Activity activity;

    private Mode mode = Mode.MODE_INACTIVE;
    private SettingsTab settingsTab;
    private byte[] id;
    private String idBase64;
    private AlertDialog saveCardDialog;

    private static final String scriptHost = "https://script.google.com/macros/s/", scriptExec = "/exec?";
    private static final String scriptResult = "result", scriptSuccess = "success", scriptUser = "user", scriptCard = "card";


    public MainTab() {
        // Required empty public constructor
    }

    private void note(String s) {
        Snackbar.make(view, s, Snackbar.LENGTH_LONG).show();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        activity = getActivity();
        context = getContext();
        clipboard = (ClipboardManager) context.getSystemService(Context.CLIPBOARD_SERVICE);
        nfc = NfcAdapter.getDefaultAdapter(context);

    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        view = inflater.inflate(R.layout.maintab, container, false);
        textOutput = view.findViewById(R.id.textOutput);
        CopyToClipboardBtn = view.findViewById(R.id.CopyToClipboardBtn);
        CopyToClipboardBtn.setClickable(false);
        CopyToClipboardBtn.setEnabled(false);
        CopyToClipboardBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ClipData clip = ClipData.newPlainText("UID", idBase64);
                clipboard.setPrimaryClip(clip);
                note(getString(R.string.copied_to_clip));
                CopyToClipboardBtn.setClickable(false);
                CopyToClipboardBtn.setEnabled(false);
                textOutput.setText(getString(R.string.ready));
            }
        });
        settingsTab = (SettingsTab) getFragmentManager().findFragmentById(R.id.settingstab);
        return view;
    }

    @Override
    public void onResume() {
        super.onResume();
        if (mode == Mode.MODE_ACTIVE) enableNfc();
    }

    @Override
    public void onPause() {
        super.onPause();
        disableNfc();
    }

    public void disableNfc() {
        if (nfc == null) return;
        nfc.disableReaderMode(activity);
    }

    public void enableNfc() {
        if (nfc == null) return;
        else if (!nfc.isEnabled()) {
            Toast.makeText(context, getString(R.string.enable_nfc), Toast.LENGTH_LONG).show();
            activity.finish();
        } else {
            textOutput.setText(getString(R.string.ready));
        }
        Bundle options = new Bundle();
        //options.putInt(NfcAdapter.EXTRA_READER_PRESENCE_CHECK_DELAY, 1000);
        nfc.enableReaderMode(activity,
                new NfcAdapter.ReaderCallback() {
                    @Override
                    public void onTagDiscovered(final Tag tag) {
                        // do something
                        Log.i("MAIN", "onTagDiscovered");
                        handleTag(tag);
                    }
                },
                NfcAdapter.FLAG_READER_NFC_A | NfcAdapter.FLAG_READER_SKIP_NDEF_CHECK,
                options);
    }

    private void handleTag(final Tag tag) {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                // do something
                //readUID(tag);
                makeNewCard(tag);
            }
        });
    }

    private void makeNewCard(Tag tag) {
        MifareUltralight mifareUlTag = MifareUltralight.get(tag);
        Log.i("MAIN", tag.toString());
        Log.i("MAIN", "type: " + mifareUlTag.getType());
        if (mifareUlTag == null || mifareUlTag.getType() == MifareUltralight.TYPE_UNKNOWN) {
            note(getString(R.string.unsupported_tag));
        } else {
            id = tag.getId();
            byte[] newKeyByte;
            if(settingsTab.dryrun.isChecked()){
                newKeyByte = DefaultKey.getOriginal();
            }
            else {
                newKeyByte = MifareUltralightAuth.generateKey(id, settingsTab.desKey);
                Log.i("makenewcard", "generated key: " + HexUtils.getHex(newKeyByte));
            }

            if (newKeyByte == null) {
                note(getString(R.string.key_gen_error));
                try {
                    mifareUlTag.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                return;
            }
            MifareUltralightKey newKey = new MifareUltralightKey(newKeyByte);
            try {
                mifareUlTag.connect();
            } catch (IOException e) {
                note(getString(R.string.write_tag_err));
            }
            if (MifareUltralightAuth.authenticate(mifareUlTag, DefaultKey.getBytes()) == AuthRetCode.AUTHENTICATED) {
                if (!MifareUltralightAuth.WriteMifareUltralightKeyAndRestrict(mifareUlTag, newKey.getBytes())) {
                    note(getString(R.string.write_tag_err));
                    try {
                        mifareUlTag.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    return;
                }
                cardProgrammed();
                note(getString(R.string.new_tag_ok));
                try {
                    mifareUlTag.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            } else if (MifareUltralightAuth.authenticate(mifareUlTag, newKey.getOriginal()) == AuthRetCode.AUTHENTICATED) {
                /*if(settingsTab.restore()) {
                    if (!MifareUltralightAuth.WriteMifareUltralightKeyAndRestore(mifareUlTag, DefaultKey.getBytes()))
                        note(getString(R.string.write_tag_err));
                    else note(getString(R.string.tag_restored));
                }
                else*/
                note(getString(R.string.tag_already_prog));
                cardProgrammed();
                try {
                    mifareUlTag.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            } else {
                note(getString(R.string.unknown_key));
                //cardProgrammed();
            }
        }
    }

    public void cardProgrammed() {

        idBase64 = Base64.encodeToString(id, Base64.URL_SAFE);
        final String idHex = HexUtils.getHex(id);
        textOutput.setText(idBase64);
        CopyToClipboardBtn.setClickable(true);
        CopyToClipboardBtn.setEnabled(true);
        saveCardDialog = new AlertDialog.Builder(context).create();
        saveCardDialog.setTitle(getString(R.string.associate_card_title));
        saveCardDialog.setMessage(getString(R.string.associate_card_msg));

        final EditText userText = new EditText(context);
        userText.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_CAP_CHARACTERS | InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD);
        userText.setHint(getString(R.string.card_user_hint));
        saveCardDialog.setView(userText);
        // 04d2a6828a6c80
        saveCardDialog.setButton(AlertDialog.BUTTON_POSITIVE, getString(R.string.confirm),
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        String userId = userText.getText().toString();

                        final JSONObject payload = new JSONObject();
                        try {
                            payload.put("cmd", "addCard");
                            payload.put("admin", settingsTab.adminId);
                            payload.put("user", userId);
                            payload.put("card", idHex);
                            payload.put("type", "ULC");

                            String payloadStr = payload.toString();
                            KeyStore keyStore = KeyStore.getInstance(SettingsTab.ANDROID_KEYSTORE);
                            keyStore.load(null);
                            SecretKey signKey =  (SecretKey) keyStore.getKey(SettingsTab.hmacKeyAlias, null);
                            Mac mac = Mac.getInstance(KeyProperties.KEY_ALGORITHM_HMAC_SHA256);
                            MessageDigest.getInstance(KeyProperties.DIGEST_SHA256);
                            mac.init(signKey);
                            final String param = "hmac=" + Base64.encodeToString(mac.doFinal(payloadStr.getBytes()), Base64.URL_SAFE);
                            Log.i("MAIN", "param: " + param + " payload: " + payloadStr);
                            postRequest(param, payloadStr);
                        }
                        catch (Exception e){
                            e.printStackTrace();
                        }

                    }
                }
        );

        saveCardDialog.setButton(AlertDialog.BUTTON_NEGATIVE, getString(R.string.cancel),
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {

                    }
                }
        );

        saveCardDialog.show();
    }

    protected void postRequest(final String param, final String payload) {
        Thread t = new Thread() {

            public void run() {
                Looper.prepare(); //For Preparing Message Pool for the child Thread



                try {
                    URL url = new URL(scriptHost + settingsTab.scriptId + scriptExec + param);
                    HttpsURLConnection conn = (HttpsURLConnection) url.openConnection();

                    conn.setDoInput(true);
                    conn.setDoOutput(true);
                    conn.setRequestMethod("POST");
                    conn.setRequestProperty("Accept", "application/json");
                    conn.setRequestProperty("Content-Type", "application/json; charset=UTF-8");


                    DataOutputStream wr = new DataOutputStream(conn.getOutputStream());
                    wr.writeBytes(payload);
                    wr.flush();
                    wr.close();

                    BufferedReader br = new BufferedReader(new InputStreamReader(conn.getInputStream()));
                    StringBuilder respString = new StringBuilder();
                    String line;
                    while ((line = br.readLine()) != null) {
                        respString.append(line);
                    }
                    br.close();
                    conn.disconnect();

                    JSONObject respJson = new JSONObject(respString.toString());
                    Log.i("HTTP", "parsed JSON: " + respJson.toString());

                    if (respJson.has(scriptResult) && respJson.get(scriptResult).toString().equals(scriptSuccess)){
                        note(getString(R.string.post_success));
                    }
                    else {
                        note(getString(R.string.post_error));
                    }

                } catch(Exception e) {
                    e.printStackTrace();
                    note(getString(R.string.post_error));
                }

                Looper.loop(); //Loop in the message queue
            }
        };

        t.start();
    }



    public void setMode(Mode _mode) {
        mode = _mode;
        if (mode == Mode.MODE_ACTIVE) {
            enableNfc();
        } else {
            disableNfc();
        }
    }

    public enum Mode {MODE_ACTIVE, MODE_INACTIVE}
}
