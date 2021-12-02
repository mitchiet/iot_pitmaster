package com.example.iotpitmaster

import android.annotation.SuppressLint
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.SeekBar

import androidx.fragment.app.Fragment
import com.example.iotpitmaster.R
import kotlinx.android.synthetic.main.content_manual.*
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch

class ManualFragment: Fragment() {

    // Connect this class with the correct layout
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?)
        : View? {

        val view = inflater.inflate(
            R.layout.content_manual,
            container,
            false)

        return view
    }

    override fun onResume() {
        super.onResume()

        val seek = blow_fan_intensity
        seek?.setOnSeekBarChangeListener(object :
            SeekBar.OnSeekBarChangeListener {

            override fun onProgressChanged(seek: SeekBar, progress: Int, fromUser: Boolean) {}
            override fun onStartTrackingTouch(p0: SeekBar?) {}
            @SuppressLint("SetTextI18n")
            override fun onStopTrackingTouch(p0: SeekBar?) {

                blow_fan_value.text = "Blow Fan: " + seek.progress + "%"

                val message = ByteArray(2)
                message[0] = 4.toByte()
                message[1] = seek.progress.toByte()

                MainActivity.m_bluetoothSocket?.outputStream?.write(message)
            }
        })

    }

}