package com.example.iotpitmaster

import android.os.Bundle
import android.provider.Settings
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch

class AutoFragment : Fragment() {

    //private val objMain = object : MainActivity() {}

    // Connect this class with the correct layout
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?)
            : View? {

        val view = inflater.inflate(
            R.layout.content_auto,
            container,
            false)

        return view
    }

//    override fun onResume() {
//        super.onResume()
//
//        GlobalScope.launch {
//            objMain.readBluetoothData()
//        }
//    }
//
//    override fun onDetach() {
//        super.onDetach()
//        GlobalScope.cancel()
//    }
}