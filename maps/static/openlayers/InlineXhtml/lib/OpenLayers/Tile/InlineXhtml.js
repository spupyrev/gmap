/* Copyright (c) 2006-2011 by OpenLayers Contributors (see authors.txt for
 * full list of contributors). Published under the Clear BSD license.
 * See http://svn.openlayers.org/trunk/openlayers/license.txt for the
 * full text of the license. */


/**
 * @requires OpenLayers/Tile.js
 * @requires OpenLayers/Request/XMLHttpRequest.js
 */

/**
 * Class: OpenLayers.Tile.InlineXhtml
 * Instances of OpenLayers.Tile.InlineXhtml are used to manage the xhtml tiles
 * used by various layers.  Create a new xhtml tile with the
 * <OpenLayers.Tile.InlineXhtml> constructor.
 *
 * Inherits from:
 *  - <OpenLayers.Tile>
 */
OpenLayers.Tile.InlineXhtml = OpenLayers.Class(OpenLayers.Tile, {

    /** 
     * Property: url 
     * {String} 
     */
    url: null,

    /** 
     * Property: request 
     * {<OpenLayers.Request.XMLHttpRequest>} 
     */ 
    request: null,

    /**
     * Property: frame
     * {DOMElement} The InlineXhtml is appended to the frame.  
     */ 
    frame: null,

    /**
    * APIProperty: xhtmlUseDocumentImportNodeForBrowsers
    * {Array} Array with browsers, which will first attempt to utilize
    * the document.importNode method rather than the manual
    * OpenLayers.Tile.InlineXhtml._importNode method.  Will be
    * ignored if a layer.xhtmlContainerId has been specified.  Applies
    * only to xhtml layers (will be ignored if isHtmlLayer is true).
    * The strings to pass to this array are the ones returned by
    * <OpenLayers.Util.getBrowserName()>.
    */
    xhtmlUseDocumentImportNodeForBrowsers: ["opera"],

    /**
    * APIProperty: xhtmlExecuteInlineScriptForBrowsers
    * {Array} Array with browsers, which require inline script in xhtml
    *  responses to be explicitly executed.  Applies only to xhtml layers 
    * (eg, isHtmlLayer is false).  The strings to pass to this array are the ones
    * returned by <OpenLayers.Util.getBrowserName()>.
    */
    xhtmlExecuteInlineScriptForBrowsers: [],

    /**
    * APIProperty: htmlExecuteInlineScriptForBrowsers
    * {Array} Array with browsers, which require inline script in html responses
    * to be explicitly executed.  Applies only to html layers (eg, isHtmlLayer is true,
   * or fallbackToHtmlLayer is true).  The strings to pass to this array are
    * the ones returned by <OpenLayers.Util.getBrowserName()>.
    */
    htmlExecuteInlineScriptForBrowsers: ["firefox", "opera", "msie", "safari"],

    /**
     * Constructor: OpenLayers.Tile.InlineXhtml
     * Constructor for a new <OpenLayers.Tile.InlineXhtml> instance.
     * 
     * Parameters:
     * layer - {<OpenLayers.Layer>} layer that the tile will go in.
     * position - {<OpenLayers.Pixel>}
     * bounds - {<OpenLayers.Bounds>}
     * url - {<String>}
     * size - {<OpenLayers.Size>}
     */
    initialize: function(layer, position, bounds, url, size) {
        OpenLayers.Tile.prototype.initialize.apply(this, arguments);

        this.id = OpenLayers.Util.createUniqueID(this.CLASS_NAME + "_");
        this.frame = OpenLayers.Util.createDiv(this.id);
        //this.frame = document.createElement('div');
        this.frame.style.overflow = this.layer.overflow || 'hidden';
        this.frame.style.position = 'absolute';
        this.frame.innerHTML = "";
        this.layer.div.appendChild(this.frame);

        if(this.layer.opacity != null) {
            OpenLayers.Util.modifyDOMElement(this.frame, null, null, null,
                                            null, null, null, 
                                            this.layer.opacity);
        };
    },

    /** 
     * APIMethod: destroy
     * nullify references to prevent circular references and memory leaks
     */
    destroy: function() {
        OpenLayers.Tile.prototype.destroy.apply(this, arguments);
        this.removeTile();
    },

    /** 
    * APIMethod: unload
    *  Destroy tile if map is resized
    */
    unload: function() {
        this.removeTile();
    },

    /** 
    * Method: removeTile
    *  Remove tile contents from layer
    */
    removeTile: function() {
        try {
            if ((this.frame != null) && (this.frame.parentNode == this.layer.div)) { 
                this.layer.div.removeChild(this.frame);
            }
        } catch (e) {
        };

        this.frame = null;
        this.url = null;
        if(this.request) {
            this.request.abort();
            //this.request.destroy();
            this.request = null;
        };
    },

    /** 
    * Method: positionFrame
    * Using the properties currenty set on the layer, position the tile correctly.
    */
    positionFrame: function() {
        if (this.layer != null) {
            OpenLayers.Util.modifyDOMElement(this.frame, 
                    null, this.position, this.size);
        };
    },

    /** 
     * Method: clear
     *  Clear the tile of any bounds/position-related data so that it can 
     *   be reused in a new location.
     */
    clear: function() {
        if (this.frame) {
            this.hide();
        }
    },
    
    /**
     * Method: draw
     * Check that a tile should be drawn.
     */
    draw:function() {
        if (OpenLayers.Tile.prototype.draw.apply(this, arguments)) {
            if (this.isLoading) {
                //if already loading, send 'reload' instead of 'loadstart'.
                this.events.triggerEvent("reload"); 
            } else {
                this.isLoading = true;
                this.events.triggerEvent("loadstart");
            }
            this.submitRequest(this.requestSuccess, this.requestFailure);
            this.hide();
        }
    },

    /** 
    * Method: submitRequest
    * Abort any pending requests and issue another request for data. 
    *
    * Input are function pointers for what to do on success and failure.
    *
    * Parameters:
    * success - {function}
    * failure - {function}
    */
    submitRequest:function(success, failure) {
        if(this.request) {
            this.request.abort();
        }

        // get request URL for tile
        var requestURL = this.layer.getURL(this.bounds);

        // add Id params if requried
        requestURL = OpenLayers.Util.urlAppend(requestURL,
                OpenLayers.Util.getParameterString(this.getUrlIdParams()));

        // add layer evaluated params if required
        requestURL = OpenLayers.Util.urlAppend(requestURL,
                OpenLayers.Util.getParameterString(this.getUrlEvaluatedParams()));

        this.request = OpenLayers.Request.GET({
            //url: this.url,
            url: requestURL,
            success: success,
            failure: failure,
            scope: this
        });
    },

    /**
    * Method: requestSuccess
    * Called on return from request succcess. Appends results into DOM
    *
    * Parameters:
    * request - {<OpenLayers.Request.XMLHttpRequest>}
    */
    requestSuccess:function(request) {

        var ImportedAsXhtml = true;

        /**
        * Function: xmlIsValid
        * This function is used to determine if an XML fragment is
        * valid (has been parsed successfully).  Due to different browser
        * implementations, there are some heuristics involved.
        * Parameters:
        * xmlDoc - {object} XML fragment
        *
        * Returns:
        * {boolean}
        */
        xmlIsValid = function(xmlDoc) {

            if (xmlDoc == null) {
                return false;
            };

            if (!xmlDoc.documentElement) {
                return false;
            };

            if (xmlDoc.parseError && xmlDoc.parseError.errorCode != 0) {
                return false;
            };

            // Heuristic checks for certain browsers
            switch(OpenLayers.Util.getBrowserName()) {
                case "firefox":
                    if (xmlDoc.documentElement.nodeName == "parsererror") {
                        return false;
                    };
                    break;
                case "safari":
                    if (xmlDoc.getElementsByTagName("parsererror").length != 0) {
                        return false;
                    };
                    break;
            };

            return true;
        };

        var xmlDoc = null;

        if (this.layer.isHtmlLayer == true) {
            // Treat content as HTML.
            ImportedAsXhtml = false;
            this.frame.innerHTML = request.responseText;
        }
        else {
            // Treat content as XHTML
            try {
                xmlDoc = request.responseXML;
            }
            catch (e) {
                xmlDoc = null;
            };

            // if responseXML is null, first try to reparse as XML
            if ((xmlDoc == null) && (request.responseText != null)) {

                var parser = new DOMParser();
                try {
                    xmlDoc = parser.parseFromString(request.responseText, "application/xhtml+xml");
                } catch(e) {
                    xmlDoc = null;
                }
            };

            if ((this.layer.fallbackToHtmlLayer == true) &&
                !xmlIsValid(xmlDoc) &&
                (request.responseText != null)) {
                    // Falling back to HTML
                    ImportedAsXhtml = false;
                    this.frame.innerHTML = request.responseText;
            }
            else {
                // Have an XML fragment
                var xmlRoot;
                var newDoc = null;

                if (this.layer.xhtmlContainerId) {
                    this.importXhtmlContainer(xmlDoc);
                } else {
                    this.importXhtml(xmlDoc);
                };
            };
        };

        this.frame.className = 'olTileImage';

        if (this.events) {
            this.events.triggerEvent("loadend"); 
        };

        //request produced with success, we can delete the request object.
        this.request = null;

        // position the frame 
        this.positionFrame();

        // show tile
        this.show();

        // execute inline script if required.
        this.executeInlineScript(ImportedAsXhtml);

    },

    /**
    * Method: requestFailure
    * Called on return from request failure
    *
    * Parameters:
    * request - {<OpenLayers.Request.XMLHttpRequest>}
    */
    requestFailure:function(request) {

        this.frame.className = 'olImageLoadError';

        // remove any existing tile contents
        while (this.frame.firstChild) {
            this.frame.removeChild(this.frame.firstChild);
        };

        //this.request.destroy();
        this.request = null;

        // position the frame
        this.positionFrame();

    },

    /**
    * Method: clone
    *
    * Parameters:
    * obj - {<OpenLayers.Tile.InlineXhtml>} The tile to be cloned
    *
    * Returns:
    * {<OpenLayers.Tile.InlineXhtml>} An exact clone of this <OpenLayers.Tile.InlineXhtml>
    */
    clone: function (obj) {
        if (obj == null) {
            obj = new OpenLayers.Tile.InlineXhtml(
                this.layer, this.position, this.bounds, this.url, this.size);
        };

        //pick up properties from superclass
        obj = OpenLayers.Tile.prototype.clone.apply(this, [obj]);

        return obj;
    },

    /**
    * Method: getUrlIdparams
    * This function is used to get the map, layer and tile id's
    * May be deprecated in the future as the getUrlEvaluatedParams can
    * be used to obtain the same data.
    * Returns:
    * {object} hashtable of map, layer and tile id's
    */
    getUrlIdParams: function() {
        if (this.layer.sendIdParams) {
            var sendparams = {};

            try {
                sendparams.OPENLAYERS_MAP_ID = this.layer.map.id || '';
            } catch (e) {
            };

            try {
                sendparams.OPENLAYERS_MAP_DIV_ID = this.layer.map.div.id || '';
            } catch (e) {
            };

            try {
                sendparams.OPENLAYERS_LAYER_ID = this.layer.id || '';
            } catch (e) {
            };

            try {
                sendparams.OPENLAYERS_TILE_ID = this.id || '';
            } catch (e) {
            };

            return sendparams;
        }
        else {
            return null;
        }
    },

    /**
    * Method: getUrlEvaluatedParams
    * This function is used to get any evaluated parameters specified in the layer
    *
    * Returns:
    * {object} hashtable of url parameters
    */
    getUrlEvaluatedParams: function() {
        var params = this.layer.evaluatedParams;
        var sendparams = {};

        try {
            if (params)
                for (var i in params) {
                    if (i != null) {
                        var param_value;
                        try {
                            param_value = eval(params[i]);
                        }
                        catch(e) {
                            param_value = '';
                        };
                        sendparams[i] = param_value;
                    }
                }
        } catch (e) {
            sendparams = null;
        };

        return sendparams;
    },

    /**
    * Method: importXhtml
    * This function is used to import the XML response into the document.
    *
    * Parameters:
    * content - {XML document} xml node to be imported
    */
    importXhtml: function(content) {
        var xmlDoc = content;
        var xmlRoot = xmlDoc.documentElement;
        var newDoc;

        // import xmlRoot into main document
        if (OpenLayers.Util.indexOf(
                this.xhtmlUseDocumentImportNodeForBrowsers,
                OpenLayers.Util.getBrowserName()) != -1) {
            try {
                // try  document.importNode to import newDoc into document
                newDoc = document.importNode(xmlRoot, true); 
            }
            catch (e) {
                try {
                    // try document.adoptNode if document.importNode() threw exception
                    newDoc = document.adoptNode(xmlRoot, true);
                }
                catch (e) {
                    // give up, use manual this._importNode
                    newDoc = this._importNode(xmlRoot, true);
                }
            }
        }
        else {
            // use manual this._importNode to import newDoc into document
            newDoc = this._importNode(xmlRoot, true);
        };

        if (newDoc != null){
            // remove any existing tile contents
            while (this.frame.firstChild) {
                this.frame.removeChild(this.frame.firstChild);
            };

            // add new contents to tile
            this.frame.appendChild(newDoc);

            newDoc = null;
        }
    },

    /**
    * Method: importXhtmlContainer
    * This function is used to import childnodes of a containing element
    * from the XML response into the document.
    *
    * Parameters:
    * content - {XML document} xml node to be imported
    */
    importXhtmlContainer: function(content) {
        var xmlRoot = content.documentElement;
        var container = this._getElementById(xmlRoot, this.layer.xhtmlContainerId);

        // remove any existing tile contents
        while (this.frame.firstChild) {
            this.frame.removeChild(this.frame.firstChild);
        };

        // Add new contents to tile
        if (container) {
            var containerDoc = container.node;
            var xmlnsArray = container.nsResolver;

            // only add child nodes of container
            if (containerDoc.childNodes) {
                for (var i = 0; i < containerDoc.childNodes.length; i++) {
                    var newChildNode = this._importNode(containerDoc.childNodes[i], true, xmlnsArray);
                    this.frame.appendChild(newChildNode);
                }
            }
        }
    },

    /**
    * Method: executeInlineScript
    * This function is used to manually execute script elements in the
    * imported content if required for the particular browser.
    */
    executeInlineScript: function(ImportedAsXhtml) {

        if ((ImportedAsXhtml == true) &&
            OpenLayers.Util.indexOf(
                this.xhtmlExecuteInlineScriptForBrowsers, 
                OpenLayers.Util.getBrowserName()) != -1) {
                this.executeScriptInDiv(this.frame);
        }
        else if ((ImportedAsXhtml == false) &&
                OpenLayers.Util.indexOf(
                    this.htmlExecuteInlineScriptForBrowsers,
                    OpenLayers.Util.getBrowserName()) != -1) {
                    this.executeScriptInDiv(this.frame);
        };
    },

    /**
    * Method: executeScriptInDiv
    * This function is used to traverse a document node and 
    * execute script elements.
    */
    executeScriptInDiv: function(node) {
        if (node.nodeType == document.ELEMENT_NODE) {
            if (node.nodeName.toLowerCase() == 'script') {
                if (node.text) {
                    window.setTimeout(node.text,1);
                } else if (node.childNodes) {
                    var run_script = '';
                    for (var i = 0; i < node.childNodes.length; i++) {
                        if ((node.childNodes[i].nodeType == document.TEXT_NODE) ||
                            (node.childNodes[i].nodeType == document.CDATA_SECTION_NODE)) {
                            run_script = run_script + node.childNodes[i].nodeValue;
                        };
                    };
                    window.setTimeout(run_script,1);
                };
            };
            if (node.childNodes) {
                // recursively scan child nodes
                for (var i = 0; i < node.childNodes.length; i++) {
                    this.executeScriptInDiv(node.childNodes[i]);
                }
            }
        }
    },

    /**
    * Method: _getElementById
    * This function is used to manually traverse an xhtml document to return a node
    *    and associated namespaces where there is a match on the passed id.
    *
    * Parameters:
    * node - {XML document} xml node to be traversed
    * id - {string}  the id of the container element
    * xmlnsArray - {Array} associative array of current defined namespace prefixes and URI's for this XML node
    *
    * Returns:
    * {object; node, nsResolver}
    *    node: {XML document} xml node with matching id attribute
    *    nsResolver: {Arrary} associative array of current defined namespace prefixes and URI's for this returned node
    *
    * Future refactoring notes:
    *    Consider use of xpath processing for compliant browsers;
    *        eg, xmlDoc.evaluate("//*[@id='id1']", xmlDoc, nsResolver, XPathResult.ANY_TYPE, null), etc
    *        as an alternative to _getElementById
    */
    _getElementById: function(node, id, xmlnsArray) {

        var foundElement = false;

        /**
        * Function: getNs
        * This function is used to retrieve the namespace URI for a given
        * namespace prefix that has previously been stored in the xmlnsArray.
        *
        * Parameters:
        * prefix - {String} namespace prefix
        *
        * Returns:
        * {String}
        */
        getNs = function(prefix) {
            if (!xmlnsArray) {
                return null;
            } 
            else if (xmlnsArray[prefix]) {
                return xmlnsArray[prefix];
            }
            else {
                return null;
            }
        };

        /**
        * Function: setNs
        * This function is used to store a namespace prefix and URI in the
        * associative array xmlnsArray.
        *
        * Parameters:
        * prefix - {String} namespace prefix
        * ns - {String} namespace URI
        */
        setNs = function(prefix, ns) {
            if (!xmlnsArray) {
                xmlnsArray = [];
            };
            xmlnsArray[prefix] = ns;
        };

        // find the node type to import
        switch (node.nodeType) {
            case document.ELEMENT_NODE:

                // first, scan this element for any namespace attributes
                if (node.attributes) {
                    for (var i = 0; i < node.attributes.length; i++) {
                        if (node.attributes[i].nodeName == 'xmlns') {
                            setNs('xmlns', node.getAttribute(node.attributes[i].nodeName));
                        }
                        else {
                            var split = node.attributes[i].nodeName.indexOf(':');
                            if (split > 0) {
                                var prefix = node.attributes[i].nodeName.substring(0, split);
                                var local = node.attributes[i].nodeName.substring(split + 1);
                                if (prefix == 'xmlns') {
                                    var attr = node.getAttribute(node.attributes[i].nodeName);
                                    if (!attr) {
                                        attr = node.getAttribute(local); 
                                    };
                                    setNs(local, attr);
                                }
                            }
                        };
                        if ((node.attributes[i].nodeName == "id") &&
                                (node.getAttribute(node.attributes[i].nodeName) == id)) {
                            foundElement = true;
                        }
                    }
                };

                // return this node if it is the element with passed id
                if (foundElement) {
                    return { node: node, nsResolver: xmlnsArray };
                } else {
                    // otherwise, scan child nodes for element
                    if (node.childNodes) {
                        // recursively scan child nodes
                        for (var i = 0; i < node.childNodes.length; i++) {
                            var newNode = this._getElementById(node.childNodes[i], id, xmlnsArray);
                            if (newNode) {
                                return newNode;
                            }
                        }
                    }
                };

                return null;
                break;
        }
    },

    /**
    * Method: _importNode
    * This function is used to manually import an xhtml fragment into the
    *    document.  Used when the inbuilt document.importNode is unreliable.
    *
    * Parameters:
    * node - {XML document} xml node to be traversed
    * deep - {boolean}  indicates whether the children of the node need to be imported
    * xmlnsArray - {Array} associative array of current defined namespace prefixes and URI's for this XML node
    *
    * Returns:
    * {document.element}
    */
    _importNode: function(node, deep, xmlnsArray) {

        /**
        * Function: getNs
        * This function is used to retrieve the namespace URI for a given
        * namespace prefix that has previously been stored in the xmlnsArray.
        *
        * Parameters:
        * prefix - {String} namespace prefix
        *
        * Returns:
        * {String}
        */
        getNs = function(prefix) {
            if (!xmlnsArray) {
                return null;
            } 
            else if (xmlnsArray[prefix]) {
                return xmlnsArray[prefix];
            }
            else {
                return null;
            }
        };

        /**
        * Function: setNs
        * This function is used to store a namespace prefix and URI in the
        * associative array xmlnsArray.
        *
        * Parameters:
        * prefix - {String} namespace prefix
        * ns - {String} namespace URI
        */
        setNs = function(prefix, ns) {
            if (!xmlnsArray) {
                xmlnsArray = [];
            };
            xmlnsArray[prefix] = ns;
        };

        // find the node type to import
        switch (node.nodeType) {
            case document.ELEMENT_NODE:

                // first, scan this element for any namespace attributes
                if (node.attributes) {
                    for (var i = 0; i < node.attributes.length; i++) {
                        if (node.attributes[i].nodeName == 'xmlns') {
                            setNs('xmlns', node.getAttribute(node.attributes[i].nodeName));
                        }
                        else {
                            var split = node.attributes[i].nodeName.indexOf(':');
                            if (split > 0) {
                                // this attribute is using a namespace prefix
                                var prefix = node.attributes[i].nodeName.substring(0, split);
                                var local = node.attributes[i].nodeName.substring(split + 1);
                                if (prefix == 'xmlns') {
                                    var attr = node.getAttribute(node.attributes[i].nodeName);
                                    if (!attr) {
                                        attr = node.getAttribute(local); 
                                    };
                                    setNs(local, attr);
                                }
                            }
                        }
                    }
                };

                var xmlns = getNs('xmlns');

                // create new document element
                var newNode;
                if (node.nodeName.indexOf(':') != -1) {
                    // inline xhtml node
                    var nSNode = node.nodeName.split(':');
                    newNode = document.createElementNS(getNs(nSNode[0]), node.nodeName);
                }
                else {
                    if (xmlns == null) { 
                        newNode = document.createElement(node.nodeName);
                    }
                    else {
                        newNode = document.createElementNS(xmlns, node.nodeName);
                         }
                };

                // add node attributes
                if (node.attributes) {
                    for (var i = 0; i < node.attributes.length; i++) {
                        if (node.attributes[i].nodeName != 'xmlns') {
                            var split = node.attributes[i].nodeName.indexOf(':');
                            if (split > 0) {
                                // this attribute is using a namespace prefix
                                var prefix = node.attributes[i].nodeName.substring(0, split);
                                var local = node.attributes[i].nodeName.substring(split + 1);
                                if (prefix != 'xmlns') {
                                    var attr = node.getAttribute(node.attributes[i].nodeName);
                                    if (!attr) {
                                        attr = node.getAttribute(local);
                                    };
                                    var nsURI = getNs(prefix);
                                    try {
                                        newNode.setAttributeNS(nsURI,
                                                node.attributes[i].nodeName,
                                                attr);
                                    } catch(e) {
                                        // Possibly non-declared namespace for this attribute
                                        if (!nsURI) {
                                            newNode.setAttribute(node.attributes[i].nodeName, attr);
                                        }
                                    }
                                }
                            }
                            else if (xmlns != null) {
                                // a namespace is already set for this element
                                newNode.setAttributeNS(null,
                                        node.attributes[i].nodeName,
                                        node.getAttribute(node.attributes[i].nodeName));
                            }
                            else {
                                // no namespace currently set for this element
                                newNode.setAttribute(node.attributes[i].nodeName, 
                                        node.getAttribute(node.attributes[i].nodeName));
                            }
                        }
                    }
                };

                // add child nodes if required
                if (deep && node.childNodes) {
                    // recursively add child nodes
                    for (var i = 0; i < node.childNodes.length; i++) {
                        newNode.appendChild(this._importNode(node.childNodes[i], deep, xmlnsArray));
                    }
                };

                return newNode;
                break;
            case document.TEXT_NODE:
                return document.createTextNode(node.nodeValue);
                break;
            case document.CDATA_SECTION_NODE:
                return document.createTextNode(node.nodeValue);
                break;
            case document.COMMENT_NODE:
               return document.createComment(node.nodeValue);
               break;
        }
    },

    /** 
     * Method: show
     * Show the tile by showing its frame.
     */
    show: function() {
        this.frame.style.display = '';
    },
    
    /** 
     * Method: hide
     * Hide the tile by hiding its frame.
     */
    hide: function() {
        this.frame.style.display = 'none';
    },

    CLASS_NAME: "OpenLayers.Tile.InlineXhtml"
  }
);

// document element constants used by _getElementById and _importNode
if (!document.ELEMENT_NODE) {
    document.ELEMENT_NODE = 1;
    document.ATTRIBUTE_NODE = 2;
    document.TEXT_NODE = 3;
    document.CDATA_SECTION_NODE = 4;
    document.ENTITY_REFERENCE_NODE = 5;
    document.ENTITY_NODE = 6;
    document.PROCESSING_INSTRUCTION_NODE = 7;
    document.COMMENT_NODE = 8;
    document.DOCUMENT_NODE = 9;
    document.DOCUMENT_TYPE_NODE = 10;
    document.DOCUMENT_FRAGMENT_NODE = 11;
    document.NOTATION_NODE = 12;
};
