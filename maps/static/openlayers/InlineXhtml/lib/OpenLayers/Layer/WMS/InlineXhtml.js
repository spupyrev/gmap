/* Copyright (c) 2006-2011 by OpenLayers Contributors (see authors.txt for
 * full list of contributors). Published under the Clear BSD license.
 * See http://svn.openlayers.org/trunk/openlayers/license.txt for the
 * full text of the license. */


/**
 * @requires OpenLayers/Layer/WMS.js
 * @requires OpenLayers/Tile/InlineXhtml.js
 */

/**
 * Class: OpenLayers.Layer.WMS.InlineXhtml
 * Instances of OpenLayers.Layer.WMS.InlineXhtml are used to retrieve data from OGC
 * Web Mapping Services where output is XHTML, eg, SVG, etc.
 * Create a new WMS layer with the <OpenLayers.Layer.WMS.InlineXhtml> constructor.
 *
 * Inherits from:
 *  - <OpenLayers.Layer.WMS>
 */
OpenLayers.Layer.WMS.InlineXhtml = OpenLayers.Class(OpenLayers.Layer.WMS, {

    /**
    * Property: isHtmlLayer
    * {boolean} If true, the incomming content will be treated as html
    * and the browsers html parser will be used.  If false (default),
    * the content will first be treated (parsed) as xhtml.
    * Applied to Layer tiles.
    * Defaults to; false.
    */ 
    isHtmlLayer: false,

    /**
    * Property: fallbackToHtmlLayer
    * {boolean} If true (and isHtmlLayer is false), the 
    * incomming content will be treated as html should the
    * initial xhtml parsing fail.  Only applies when isHtmlLayer is
    * false.  Applied to Layer tiles
    * Defaults to; false.
    */
    fallbackToHtmlLayer: false,

    /**
    * Property: overflow
    * {DOMElement} Overflow style for layer tiles.
    * Applied to Layer tiles.
    *Defauls to: "hidden".
    */ 
    overflow: 'hidden',

    /**
    * Property: pointerEvents
    * {DOMElement} Pointer-Events style for layer.
    * Applied to Layer.
    */ 
    pointerEvents: null,

    /**
    * Property: xhtmlContainerId
    * {string} String used to search xhtml content for a matching
    * element having the specified id attribute.  Only child nodes of that
    * element will be imported into the tile.
    * Applied to Layer tiles.
    */ 
    xhtmlContainerId: null,

    /**
    * Property: sendIdParams
    * {boolean} indicates whether map, layer and tile Id
    * parameters are to be passed to WMS request.
    * Applied to Layer tiles.
    */ 
    sendIdParams: false,

    /**
    * Property: evaluatedParams
    * {object} hash table of parameters to be passed as
    * part of the WMS request.  Parameters are evaluated
    * prior to appending to request.  Applied to Layer tiles.
    */ 
    evaluatedParams: null,

    /**
     * Constructor: OpenLayers.Layer.WMS.InlineXhtml
     * Creates a new WMS layer object.
     *
     * Parameters:
     * name - {String} A name for the layer
     * url - {String} Base url for the WMS
     *                (e.g. http://wms.jpl.nasa.gov/wms.cgi)
     * params - {Object} An object with key/value pairs representing the
     *                   GetMap query string parameters and parameter values.
     * options - {Object} Hashtable of extra options to tag onto the layer.
     */
    initialize: function(name, url, params, options) {
        OpenLayers.Layer.WMS.prototype.initialize.apply(this, arguments);

        if (options == null && this.options.pointerEvents) {
           this.div.style.pointerEvents = this.options.pointerEvents;
        }
    },

    /**
     * Method: addTile
     * addTile creates a tile, initializes it and adds it to the
     * layer div.
     *
     * Parameters:
     * bounds - {<OpenLayers.Bounds>}
     * position - {<OpenLayers.Pixel>}
     *
     * Returns:
     * {<OpenLayers.Tile.InlineXhtml>} The added OpenLayers.Tile.InlineXhtml
     */
    addTile: function(bounds,position) {
        return new OpenLayers.Tile.InlineXhtml(
            this, position, bounds, this.url, this.tileSize);
    },

    CLASS_NAME: 'OpenLayers.Layer.WMS.InlineXhtml'
});
