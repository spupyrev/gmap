/* Copyright (c) 2006-2011 by OpenLayers Contributors (see authors.txt for
 * full list of contributors). Published under the Clear BSD license.
 * See http://svn.openlayers.org/trunk/openlayers/license.txt for the
 * full text of the license. */

 
/**
 * @requires OpenLayers/Layer.js
 * @requires OpenLayers/Tile/InlineXhtml.js
 */

/**
 * Class: OpenLayers.Layer.ScalableInlineXhtml
 * Instances of OpenLayers.Layer.ScalableInlineXhtml are used to display data from a 
 * scalable xhtml service or file as a map layer.  Create a new xhtml layer with the
 * <OpenLayers.Layer.ScalableInlineXhtml> constructor.  Inherits from <OpenLayers.Layer>.
 */
OpenLayers.Layer.ScalableInlineXhtml = OpenLayers.Class(OpenLayers.Layer, {

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
    * parameters are to be passed as the url request.
    * Applied to Layer tiles.
    */ 
    sendIdParams: false,

    /**
    * Property: evaluatedParams
    * {object} hash table of parameters to be passed as
    * part of the url request.  Parameters are evaluated
    * prior to appending to request.  Applied to Layer tiles.
    */ 
    evaluatedParams: null,

    /**
     * Property: url
     * {String} URL of the image to use
     */
    url: null,

    /**
     * Property: extent
     * {<OpenLayers.Bounds>} The image bounds in map units.  This extent will
     *     also be used as the default maxExtent for the layer.  If you wish
     *     to have a maxExtent that is different than the image extent, set the
     *     maxExtent property of the options argument (as with any other layer).
     */
    extent: null,
    
    /**
     * Property: size
     * {<OpenLayers.Size>} The image size in pixels as original unscaled where
     * the viewport corresponds to the specified extent.
     */
    size: null,

    /**
     * Property: tile
     * {<OpenLayers.Tile.InlineXhtml>}
     */
    tile: null,

    /**
     * Property: aspectRatio
     * {Float} The ratio of height/width represented by a single pixel in the
     * graphic
     */
    aspectRatio: null,

    /**
     * Constructor: OpenLayers.Layer.ScalableInlineXhtml
     * Create a new scalable xhtml layer
     *
     * Parameters:
     * name - {String} A name for the layer.
     * url - {String} Relative or absolute path to the xhtml service or file
     * extent - {<OpenLayers.Bounds>} The extent represented by the image
     * size - {<OpenLayers.Size>} The size (in pixels) of the image (optional)
     * options - {Object} Hashtable of extra options to tag onto the layer
     */
    initialize: function(name, url, extent, size, options) {
        this.url = url;
        this.extent = extent;
        this.maxExtent = extent;
        this.size = size;
        OpenLayers.Layer.prototype.initialize.apply(this, [name, options]);

        if (this.size != null) {
            this.aspectRatio = (this.extent.getHeight() / this.size.h) /
                               (this.extent.getWidth() / this.size.w);
        } else {
            this.aspectRatio = this.extent.getHeight() / this.extent.getWidth();
        };

        if (options.id == null && this.options.pointerEvents) {
           this.div.style.pointerEvents = this.options.pointerEvents;
        }

    },    

    /**
     * Method: destroy
     * Destroy this layer
     */
    destroy: function() {
        if (this.tile) {
            this.removeTileMonitoringHooks(this.tile);
            this.tile.destroy();
            this.tile = null;
        }
        OpenLayers.Layer.prototype.destroy.apply(this, arguments);
    },
    
    /**
     * Method: clone
     * Create a clone of this layer
     *
     * Paramters:
     * obj - {Object} An optional layer (is this ever used?)
     *
     * Returns:
     * {<OpenLayers.Layer.ScalableInlineXhtml>} An exact copy of this layer
     */
    clone: function(obj) {
        
        if(obj == null) {
            obj = new OpenLayers.Layer.ScalableInlineXhtml(this.name,
                                               this.url,
                                               this.extent,
                                               this.size,
                                               this.getOptions());
        }

        //get all additions from superclasses
        obj = OpenLayers.Layer.prototype.clone.apply(this, [obj]);

        // copy/set any non-init, non-simple values here

        return obj;
    },    
    
    /**
     * APIMethod: setMap
     * 
     * Parameters:
     * map - {<OpenLayers.Map>}
     */
    setMap: function(map) {
        /**
         * If nothing to do with resolutions has been set, assume a single
         * resolution determined by ratio*extent/size - if an image has a
         * pixel aspect ratio different than one (as calculated above), the
         * image will be stretched in one dimension only.
         */
        if( this.options.maxResolution == null &&
            this.size != null) {
            this.options.maxResolution = this.aspectRatio *
                                         this.extent.getWidth() /
                                         this.size.w;
        }
        OpenLayers.Layer.prototype.setMap.apply(this, arguments);
    },

    /** 
     * Method: moveTo
     * Create the tile for the image or resize it for the new resolution
     * 
     * Parameters:
     * bounds - {<OpenLayers.Bounds>}
     * zoomChanged - {Boolean}
     * dragging - {Boolean}
     */
    moveTo:function(bounds, zoomChanged, dragging) {
        OpenLayers.Layer.prototype.moveTo.apply(this, arguments);

        var firstRendering = (this.tile == null);

        if(zoomChanged || firstRendering) {

            //determine new tile size
            this.setTileSize();

            //determine new position (upper left corner of new bounds)
            var ul = new OpenLayers.LonLat(this.extent.left, this.extent.top);
            var ulPx = this.map.getLayerPxFromLonLat(ul);

            if(firstRendering) {
                //create the new tile
                this.tile = new OpenLayers.Tile.InlineXhtml(this, ulPx, this.extent, 
                                                      null, this.tileSize);
                this.addTileMonitoringHooks(this.tile);
                this.tile.draw();
            } else {
                //just resize the tile and set it's new position
                this.tile.size = this.tileSize.clone();
                this.tile.position = ulPx.clone();
                // position the frame (content is scalable)
                this.tile.positionFrame();
            }
        }
    }, 

    /**
     * Set the tile size based on the map size.
     */
    setTileSize: function() {
        var tileWidth = this.extent.getWidth() / this.map.getResolution();
        var tileHeight = this.extent.getHeight() / this.map.getResolution();
        this.tileSize = new OpenLayers.Size(tileWidth, tileHeight);
    },

    /** 
     * Method: addTileMonitoringHooks
     * This function takes a tile as input and adds the appropriate hooks to 
     *     the tile so that the layer can keep track of the loading tiles.
     * 
     * Parameters: 
     * tile - {<OpenLayers.Tile>}
     */
    addTileMonitoringHooks: function(tile) {
        tile.onLoadStart = function() {
            this.events.triggerEvent("loadstart");
        };
        tile.events.register("loadstart", this, tile.onLoadStart);
      
        tile.onLoadEnd = function() {
            this.events.triggerEvent("loadend");
        };
        tile.events.register("loadend", this, tile.onLoadEnd);
        tile.events.register("unload", this, tile.onLoadEnd);
    },

    /** 
     * Method: removeTileMonitoringHooks
     * This function takes a tile as input and removes the tile hooks 
     *     that were added in <addTileMonitoringHooks>.
     * 
     * Parameters: 
     * tile - {<OpenLayers.Tile>}
     */
    removeTileMonitoringHooks: function(tile) {
        tile.unload();
        tile.events.un({
            "loadstart": tile.onLoadStart,
            "loadend": tile.onLoadEnd,
            "unload": tile.onLoadEnd,
            scope: this
        });
    },
    
    /**
     * APIMethod: setUrl
     * 
     * Parameters:
     * newUrl - {String}
     */
    setUrl: function(newUrl) {
        this.url = newUrl;
        this.tile.draw();
    },

    /** 
     * APIMethod: getURL
     * Return the base url for the image or xhtml service.  The tile class
     * may add additional parameters.
     * 
     * Parameters:
     * bounds - {<OpenLayers.Bounds>}
     */
    getURL: function(bounds) {
        return this.url;
    },

    CLASS_NAME: "OpenLayers.Layer.ScalableInlineXhtml"
});
