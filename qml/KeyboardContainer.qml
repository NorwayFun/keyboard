/*
 * Copyright 2013 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import QtQuick.Window 2.0
import "languages/"
import "keys/"

Item {
    id: panel

    property int keyWidth: 0
    property int keyHeight: 0

    property Item activeKeypad: characterKeypadLoader.item
    property string activeKeypadState: "NORMAL"
    property string characterKeypadSource: ""
    property string symbolKeypadSource: activeKeypad ? activeKeypad.symbols : ""


    state: "CHARACTERS"

    property variant supportedLocales: ["en", "de", "es", "fr", "zh", "pt"]

    function localeIsSupported(locale)
    {
        return (supportedLocales.indexOf( locale ) > -1)
    }

    function loadLayout(layoutId)
    {
        var locale = Qt.locale().name.slice(0,2).toLowerCase();

        if (layoutId === "number")
            characterKeypadSource = "languages/Keyboard_numbers.qml";
        if (layoutId === "phonenumber")
            characterKeypadSource = "languages/Keyboard_telephone.qml";
        if (layoutId === "email")
            if (localeIsSupported(locale))
                characterKeypadSource = "languages/"+locale+"/Keyboard_"+locale+"_email.qml";
        if (layoutId === "url")
            if (localeIsSupported(locale))
                characterKeypadSource = "languages/"+locale+"/Keyboard_"+locale+"_url_search.qml";
        if (layoutId === "en_us")
            characterKeypadSource = "languages/en_us/Keyboard_en.qml";
        if (layoutId === "es")
            characterKeypadSource = "languages/es/Keyboard_es.qml";
        if (layoutId === "pt")
            characterKeypadSource = "languages/pt/Keyboard_pt.qml";
        if (layoutId === "de")
            characterKeypadSource = "languages/de/Keyboard_de.qml";
        if (layoutId === "fr")
            characterKeypadSource = "languages/fr/Keyboard_fr.qml";
        if (layoutId === "zh_cn_pinyin")
            characterKeypadSource = "languages/zh/Keyboard_zh_pinyin.qml";
    }

    onCharacterKeypadSourceChanged: {
        panel.state = "CHARACTERS"
    }

    Component.onCompleted: symbolKeypadSource = activeKeypad.symbols;

    Loader {
        id: characterKeypadLoader
        objectName: "characterKeyPadLoader"
        anchors.fill: parent
        asynchronous: false
        source: panel.state === "CHARACTERS" ? characterKeypadSource : symbolKeypadSource
        onLoaded: activeKeypadState = "NORMAL"
    }
    ExtendedKeysSelector {
        id: extendedKeysSelector
        objectName: "extendedKeysSelector"
        anchors.fill: parent
    }

    function closeExtendedKeys()
    {
        extendedKeysSelector.closePopover();
    }

    states: [
        State {
            name: "CHARACTERS"
        },
        State {
            name: "SYMBOLS"
        }
    ]

}

