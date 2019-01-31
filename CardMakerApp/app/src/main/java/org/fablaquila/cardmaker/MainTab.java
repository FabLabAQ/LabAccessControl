package org.fablaquila.cardmaker;
import org.fablaquila.cardmaker.MifareUltralightAuth.AuthRetCode;

import android.support.v4.app.Fragment;
import android.os.Bundle;
import android.util.Base64;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.Switch;
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
import java.lang.Runnable;
import java.io.IOException;

public class MainTab extends Fragment {

    private TextView textOutput;
    private Button CopyToClipboardBtn;
    private View view;
    private ClipboardManager clipboard;
    private Context context;
    private NfcAdapter nfc;
    private Activity activity;
    public enum Mode {MODE_ACTIVE, MODE_INACTIVE};
    private Mode mode = Mode.MODE_INACTIVE;
    private SettingsTab settingsTab;
    private byte[] id;
    private String idBase64;



    private final MifareUltralightKey DefaultKey = new MifareUltralightKey(new byte[] {'B','R','E','A','K','M','E','I','F','Y','O','U','C','A','N','!'});

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
            public void onClick(View v)
            {
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
        }
        else {
            textOutput.setText(getString(R.string.ready));
        }
        Bundle options = new Bundle();
        //options.putInt(NfcAdapter.EXTRA_READER_PRESENCE_CHECK_DELAY, 1000);
        nfc.enableReaderMode(activity,
                new NfcAdapter.ReaderCallback() {
                    @Override
                    public void onTagDiscovered(final Tag tag) {
                        // do something
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
        if (mifareUlTag == null || mifareUlTag.getType() != MifareUltralight.TYPE_ULTRALIGHT_C) {
            note(getString(R.string.unsupported_tag));
        }
        else {
            id = tag.getId();
            byte[] newKeyByte = MifareUltralightAuth.generateKey(id, settingsTab.masterKey);
            //Log.d("makenewcard", "generated key: " + HexUtils.getHex(newKeyByte));
            if (newKeyByte == null) {
                note(getString(R.string.key_gen_error));
                try { mifareUlTag.close(); } catch (IOException e) { }
                return;
            }
            MifareUltralightKey newKey = new MifareUltralightKey(newKeyByte);
            try {
                mifareUlTag.connect();
            } catch (IOException e) {
                note(getString(R.string.write_tag_err));
            }
            if (MifareUltralightAuth.authenticate(mifareUlTag, DefaultKey.getBytes()) == AuthRetCode.AUTHENTICATED){
                if (!MifareUltralightAuth.WriteMifareUltralightKeyAndRestrict(mifareUlTag, newKey.getBytes())) {
                    note(getString(R.string.write_tag_err));
                    try { mifareUlTag.close(); } catch (IOException e) { }
                    return;
                }
                idBase64 = Base64.encodeToString(id, Base64.DEFAULT);
                textOutput.setText(idBase64);
                CopyToClipboardBtn.setClickable(true);
                CopyToClipboardBtn.setEnabled(true);
                note(getString(R.string.new_tag_ok));
                try { mifareUlTag.close(); } catch (IOException e) { }
            }
            else if (MifareUltralightAuth.authenticate(mifareUlTag, newKey.getOriginal()) == AuthRetCode.AUTHENTICATED) {
                /*if(settingsTab.restore()) {
                    if (!MifareUltralightAuth.WriteMifareUltralightKeyAndRestore(mifareUlTag, DefaultKey.getBytes()))
                        note(getString(R.string.write_tag_err));
                    else note(getString(R.string.tag_restored));
                }
                else*/ note(getString(R.string.tag_already_prog));
                idBase64 = Base64.encodeToString(id, Base64.DEFAULT);
                textOutput.setText(idBase64);
                CopyToClipboardBtn.setClickable(true);
                CopyToClipboardBtn.setEnabled(true);
                try { mifareUlTag.close(); } catch (IOException e) { }
            }
            else {
                note(getString(R.string.unknown_key));
            }
        }
    }

    public void setMode(Mode _mode) {
        mode = _mode;
        if (mode == Mode.MODE_ACTIVE) {
            enableNfc();
        }
        else {
            disableNfc();
        }
    }
}